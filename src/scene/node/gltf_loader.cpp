#include <engine/scene/node/gltf_loader.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/resources_manager.hpp>
#include <engine/utils/hash.hpp>

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <list>
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>
#include <tiny_gltf.h>


using namespace std;

namespace engine {
    static tinygltf::Model load_gltf_from_file(const std::string& path, bool binary) {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        string err, warn;

        bool success =
            binary ? loader.LoadBinaryFromFile(&model, &err, &warn, path) // for .glb
                : loader.LoadASCIIFromFile(&model, &err, &warn, path); // for .gltf

        if(!warn.empty())
            slogga::stdout_log.warn("{}", warn);

        if(!err.empty())
            slogga::stdout_log.error("{}", err);

        if(!success)
            throw gltf_load_error(gltf_load_error::PARSE_ERROR, path, err);

        return model;
    }

    // BufferViews can have a byteStride of 0 if they are tightly packed, in which case we need to deduce their stride
    // given a set of vbos to deduce the stride of this function returns a map from
    static hashmap<size_t, size_t> deduce_bufview_strides(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
                                                                 const hashset<size_t>& bufviews) {
        using bufview_to_stride_t = hashmap<size_t, size_t>;
        bufview_to_stride_t bufview_to_stride;
        for (auto& [attrib_name, accessor_idx] : primitive.attributes) {
            const tinygltf::Accessor& accessor = model.accessors[accessor_idx];
            int bufview_idx = accessor.bufferView;
            if(bufviews.contains(bufview_idx)) {
                uint vec_len = accessor.type == TINYGLTF_TYPE_SCALAR ? 1 : accessor.type;

                size_t attrib_size = vec_len * gal::typeid_to_size(accessor.componentType);

                bufview_to_stride_t::iterator iterator;
                if(!bufview_to_stride.contains(bufview_idx)) {
                    auto pair = bufview_to_stride.insert({bufview_idx, 0});
                    EXPECTS(pair.second);
                    iterator = pair.first;
                } else {
                    iterator = bufview_to_stride.find(bufview_idx);
                }

                iterator->second += attrib_size;
            }
        }

        return bufview_to_stride;
    }

    static void deduce_vbo_strides(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
                                   std::vector<gal::vertex_buffer>& vbos, const hashmap<int, int>& bufview_to_vbo) {
        hashset<size_t> vbos_to_deduce_strides_of;
        for(int i = 0; i < vbos.size(); i++) {
            if(vbos[i].get_stride() == 0) {
                vbos_to_deduce_strides_of.insert(i);
            }
        }

        hashset<size_t> bufviews_to_deduce_strides_of;
        for(auto[bufview_idx, vbo_idx] : bufview_to_vbo) {
            if(vbos_to_deduce_strides_of.contains(vbo_idx)) {
                bufviews_to_deduce_strides_of.insert(bufview_idx);
            }
        }
        hashmap<size_t, size_t> strides = deduce_bufview_strides(model, primitive, bufviews_to_deduce_strides_of);

        for(auto[bufview_idx, stride] : strides) {
            size_t vbo_idx = bufview_to_vbo.find(bufview_idx)->second;
            vbos[vbo_idx].set_stride(stride);
        }
    }

    static gal::vertex_buffer make_vbo_from_bufview(const tinygltf::Model& model, int bufview_idx) {
        auto& bufview = model.bufferViews[bufview_idx];
        auto& buf = model.buffers[bufview.buffer];
        EXPECTS(bufview.target == TINYGLTF_TARGET_ARRAY_BUFFER);

        return gal::vertex_buffer(&buf.data[bufview.byteOffset], bufview.byteLength, bufview.byteStride);
    }

    static gal::index_buffer make_ibo_from_bufview(const tinygltf::Model& model, int bufview_idx, int element_typeid) {
        auto& bufview = model.bufferViews[bufview_idx];
        auto& buf = model.buffers[bufview.buffer];
        EXPECTS(bufview.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

        gal::buffer raw_data(&buf.data[bufview.byteOffset], bufview.byteLength);
        int triangle_size = gal::typeid_to_size(element_typeid) * 3;
        int triangle_count = bufview.byteLength / triangle_size;

        return gal::index_buffer(std::move(raw_data), triangle_count, element_typeid);
    }

    static gal::vertex_array get_vao_from_mesh_primitive(const tinygltf::Model& model, const tinygltf::Mesh& mesh, int primitive_idx) {
        const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];
        const tinygltf::Accessor& indices_accessor = model.accessors[primitive.indices];
        // TODO: support other modes other than TRIANGLES
        UNIMPLEMENTED(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        // TODO: currently only supporting indexed data; if primitive.indices is undefined(-1) then the primitive is non-indexed
        UNIMPLEMENTED(primitive.indices != -1);
        // TODO: currently the only types supported by gal::index_buffer are unsigned int and unsigned short; are there others we should be supporting?
        UNIMPLEMENTED(indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
            || indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);

        // TODO: currently all mesh primitives must have distinct vbos, which may cause duplication of data in VRAM. fix this.
        vector<gal::vertex_buffer> vbos;
        vector<gal::index_buffer> ibos;
        hashmap<int, int> bufview_to_vbo_map;

        using vertex_array_attrib = gal::vertex_layout::vertex_array_attrib;
        gal::vertex_layout layout;

        //populate vbos and layout.attribs
        for (auto& [attrib_name, accessor_idx] : primitive.attributes) {
            //vertex array attribute
            uint vaa =
                attrib_name == "POSITION" ? 0 :
                attrib_name == "TEXCOORD_0" ? 1 : (uint)-1;

            if(vaa != (uint)-1) {
                const tinygltf::Accessor& accessor = model.accessors[accessor_idx];
                int bufview_idx = accessor.bufferView;

                uint vbo_bind = (uint)-1;

                // TODO: every vbo contains the contents of a bufview, but this may cause the transfer of unused data to VRAM (because unused attribs might be stored in the same bufview as used ones)
                if (!bufview_to_vbo_map.contains(bufview_idx)) {
                    vbo_bind = vbos.size();
                    bufview_to_vbo_map[bufview_idx] = vbo_bind;
                    vbos.push_back(make_vbo_from_bufview(model, bufview_idx));
                }

                // TODO: support matrices: this currently only works for scalars and vec2/3/4
                uint size = accessor.type == TINYGLTF_TYPE_SCALAR ?  1 : accessor.type;

                if (vbo_bind == (uint)-1) // only lookup in the table if we didn't just add the element into it
                    vbo_bind = bufview_to_vbo_map[bufview_idx];

                vertex_array_attrib attrib { vaa, (uint)accessor.byteOffset, (uint)accessor.componentType, size, vbo_bind, accessor.normalized };
                layout.attribs.push_back(attrib);
            }
        }

        deduce_vbo_strides(model, primitive, vbos, bufview_to_vbo_map);

        // TODO: currently only 1 ibo per vao is supported; does GLTF support more? (is that how you do LOD?)
        gal::index_buffer ibo = make_ibo_from_bufview(model, indices_accessor.bufferView, indices_accessor.componentType);
        ibos.push_back(std::move(ibo));

        return gal::vertex_array(std::move(vbos), std::move(ibos), std::move(layout));
    }

    static gal::texture get_texture_from_mesh_primitive(const tinygltf::Model& model, const tinygltf::Mesh& mesh, int primitive_idx) {
        int material_idx = mesh.primitives[primitive_idx].material;
        EXPECTS(material_idx != -1);
        int texture_idx = model.materials[material_idx].pbrMetallicRoughness.baseColorTexture.index;
        EXPECTS(texture_idx != -1);
        int image_idx = model.textures[texture_idx].source;
        EXPECTS(image_idx != -1);
        const tinygltf::Image& image = model.images[image_idx];

        UNIMPLEMENTED(image.bits == 8);

        gal::texture::specification spec {
            .res = { image.width, image.height },
            .components = image.component,
            .data = image.image.data(),
            .alignment = 1,
            .repeat_wrap = true,
        };
        return gal::texture(spec);

    }

    static engine::mesh load_mesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) {
        vector<engine::primitive> primitives;
        for(size_t primitive_idx = 0; primitive_idx < mesh.primitives.size(); primitive_idx++) {
            gal::vertex_array vao = get_vao_from_mesh_primitive(model, mesh, primitive_idx);
            gal::texture texture = get_texture_from_mesh_primitive(model, mesh, primitive_idx);

            primitives.emplace_back(
                material(get_rm().get_retro_3d_shader(), get_rm().new_from<gal::texture>(std::move(texture))),
                get_rm().new_from<gal::vertex_array>(std::move(vao))
            );
        }
        return engine::mesh(std::move(primitives));
    }

    enum class colshape_load_error { NO_POSITION_ACCESSOR, POSITION_IS_NOT_VEC3 };
    using colshape_or_error = std::variant<collision_shape, colshape_load_error>;
    static colshape_or_error load_mesh_as_collision_shape(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Value& extras) {
        //TODO: multiple collision_shape primitives are currently unsupported
        UNIMPLEMENTED(mesh.primitives.size() == 1);
        const tinygltf::Primitive& primitive = mesh.primitives[0];

        const tinygltf::Accessor& indices_accessor = model.accessors[primitive.indices];
        // TODO: support other modes other than TRIANGLES for collision_shape
        UNIMPLEMENTED(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        // TODO: currently only supporting indexed data for collision_shape; if primitive.indices is undefined(-1) then the primitive is non-indexed
        UNIMPLEMENTED(primitive.indices != -1);
        // TODO: currently the only types supported for indices for collision_shape is unsigned int and unsigned short
        UNIMPLEMENTED(indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
                   || indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);

        // unfortunately blender does not support arrays of 64 bools but only up to 32

        auto load_u64_from_hex_string_extra = [&](const char* attrib_name) -> uint64_t {
            uint64_t ret = 0;
            if(extras.Has(attrib_name)) {
                tinygltf::Value attrib = extras.Get(attrib_name);
                if(!attrib.IsString()) {
                    slogga::stdout_log.warn("invalid usage of gltf extra attribute '{}': must be a string (it wasn't) containing a hex number up to 64 bits", attrib_name);
                    return 0;
                }
                std::string string = attrib.Get<std::string>();
                try {
                    ret = std::stoull(string, nullptr, 16);
                    // Exceptions from cppreference:
                    // std::invalid_argument if no conversion could be performed.
                    // std::out_of_range if the converted value would fall out of the range of the result type or if the underlying function
                } catch(std::invalid_argument& e) {
                    slogga::stdout_log.warn("invalid usage of gltf extra attribute '{}': the string could not be parsed as a hex number", attrib_name);
                    return 0;
                } catch(std::out_of_range& e) {
                    slogga::stdout_log.warn("invalid usage of gltf extra attribute '{}': the string's value could not be contained in 64 bits", attrib_name);
                    return 0;
                }
            }
            return ret;
        };

        collision_layers_bitmask is_layers = load_u64_from_hex_string_extra("is_layers");
        collision_layers_bitmask sees_layers = load_u64_from_hex_string_extra("sees_layers");
        // TODO: also load collision behaviour from gltf extras field

        int position_accessor_idx = -1;
        for (auto& [attrib_name, accessor_idx] : primitive.attributes) {
            // the only relevant attribute for collision_shape is the position
            if(attrib_name != "POSITION") continue;
            position_accessor_idx = accessor_idx;
            break;
        }


        // there must be a position accessor, otherwise there can be no collision shape
        if(position_accessor_idx == -1) return colshape_load_error::NO_POSITION_ACCESSOR;

        const tinygltf::Accessor& position_accessor = model.accessors[position_accessor_idx];

        // only vec3 are supported for position since we only support 3d collision
        if(position_accessor.type != 3) return colshape_load_error::POSITION_IS_NOT_VEC3;

        const tinygltf::BufferView& position_bufview = model.bufferViews[position_accessor.bufferView];
        const tinygltf::Buffer& position_buf = model.buffers[position_bufview.buffer];

        // get stride of the buffer containing the POSITION attrib
        size_t verts_stride = position_bufview.byteStride;
        //if it is 0 deduce it
        if(verts_stride == 0) {
            auto stride_map = deduce_bufview_strides(model, primitive, {(size_t)position_accessor.bufferView});
            verts_stride = stride_map[position_accessor.bufferView];
        }

        const void* verts_ptr = &position_buf.data[position_bufview.byteOffset];
        size_t number_of_verts = position_bufview.byteLength / verts_stride;
        stride_span<const glm::vec3> verts_span(verts_ptr, 0, verts_stride, number_of_verts);

        auto& indices_bufview = model.bufferViews[indices_accessor.bufferView];
        EXPECTS(indices_bufview.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
        auto& indices_buf = model.buffers[indices_bufview.buffer];

        if(indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            std::span<const glm::uvec3> indices_span((glm::uvec3*)&indices_buf.data[indices_bufview.byteOffset], indices_bufview.byteLength / sizeof(glm::uvec3));
            return engine::collision_shape::from_mesh(verts_span, indices_span, is_layers, sees_layers);
        } else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
            std::span<const glm::u16vec3> indices_span((glm::u16vec3*)&indices_buf.data[indices_bufview.byteOffset], indices_bufview.byteLength / sizeof(glm::u16vec3));
            return engine::collision_shape::from_mesh(verts_span, indices_span, is_layers, sees_layers);
        }
        throw std::runtime_error("given prior assertions this should be unreachable");
    }

    static glm::mat4 get_node_transform(const tinygltf::Node& n) {
        using namespace glm;

        mat4 rotation_mat = n.rotation.empty() ? mat4(1) : mat4_cast((quat)make_quat(n.rotation.data()));
        mat4 scale_mat = n.scale.empty() ? mat4(1) : scale(mat4(1), (vec3)make_vec3(n.scale.data()));
        mat4 translation_mat = n.translation.empty() ? mat4(1) : translate(mat4(1), vec3(n.translation[0], n.translation[1], n.translation[2]));
        mat4 raw_mat = n.matrix.empty() ? mat4(1) : (mat4)make_mat4(n.matrix.data());

        return translation_mat * scale_mat * rotation_mat * raw_mat;
    }

    static node_data_variant_t load_node_data(const tinygltf::Model& model, const tinygltf::Node& node) {
        if(node.mesh == -1)
            return null_node_data();
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        slogga::stdout_log.warn("loading node data for node '{}'", node.name);
        if(node.name.ends_with("-col")) {
            auto colshape_or_err = load_mesh_as_collision_shape(model, mesh, node.extras);
            if(std::holds_alternative<collision_shape>(colshape_or_err)) {
                return get_rm().new_from(std::move(std::get<collision_shape>(colshape_or_err)));
            } else {
                auto err = std::get<colshape_load_error>(colshape_or_err);
                std::string_view errstr;
                switch(err) {
                case colshape_load_error::NO_POSITION_ACCESSOR:
                    errstr = "no position vertex attribute accessor";
                    break;
                case colshape_load_error::POSITION_IS_NOT_VEC3:
                    errstr = "position vertex attribute is not vec3";
                default:
                    errstr = "(invalid error code)";
                }
                slogga::stdout_log.warn("Error while attempting to load collision shape from node \"{}\" from gltf: {}", node.name, errstr);
                return null_node_data();
            }
        } else {
            return load_mesh(model, mesh);
        }
    }

    static noderef load_node_subtree(const tinygltf::Model& model, int idx) {
        const tinygltf::Node& node = model.nodes[idx];

        node_data_variant_t node_data = load_node_data(model, node);

        glm::mat4 transform = get_node_transform(node);
        noderef root(node.name, std::move(node_data), transform);

        for(int child_idx : node.children) {
            root.add_child(load_node_subtree(model, child_idx));
        }

        return root;
    }

    engine::nodetree_blueprint load_nodetree_from_gltf(const std::string& filepath, const std::string& nodetree_name) {
        const std::string& nonempty_nodetree_name = !nodetree_name.empty() ? nodetree_name : filepath;
        bool binary = string_view(filepath).ends_with(".glb");

        if(!binary && !string_view(filepath).ends_with(".gltf"))
            throw gltf_load_error(gltf_load_error::FILE_EXTENSION_ERROR, filepath);

        const tinygltf::Model model = load_gltf_from_file(filepath, binary);

        noderef root(filepath);

        const tinygltf::Scene& scene = model.scenes.at(0);
        list<int> node_idx_queue;
        for (int node_idx : scene.nodes)
            root.add_child(load_node_subtree(model, node_idx));

        return engine::nodetree_blueprint(std::move(root), nonempty_nodetree_name);
    }

    const char* gltf_load_error::what() const noexcept {
        if(m_what.empty()) {
            switch(m_type) {
            case PARSE_ERROR:
                m_what = std::format("Failed to parse glTF file at path {};\nerror: {}", m_filepath, m_err);
                break;
            case FILE_EXTENSION_ERROR:
                m_what = std::format("File {} is not a gltf file: extension is not .glb or .gltf", m_filepath);
                break;
            default:
                m_what = std::format("(invalid gltf_load_error type; path={}, err={})", m_filepath, m_err);
            }
        }

        return m_what.c_str();
    }

}
