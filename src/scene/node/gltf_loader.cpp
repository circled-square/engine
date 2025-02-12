#include <engine/scene/node/gltf_loader.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/resources_manager.hpp>

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <unordered_map>
#include <unordered_set>
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
            throw runtime_error(std::format("Fatal error: Failed to parse glTF file at path {}", path));

        return model;
    }

    //BufferViews can have a byteStride of 0 if they are tightly packed, in which case we need to deduce their stride
    static void deduce_vbo_strides(vector<gal::vertex_buffer>& vbos, const gal::vertex_layout& vertex_layout) {
        unordered_set<size_t> vbos_to_deduce_stride_of;
        for(size_t i = 0; i < vbos.size(); i++) {
            if(vbos[i].get_stride() == 0)
                vbos_to_deduce_stride_of.insert(i);
        }

        for(const auto& attrib : vertex_layout.attribs) {
            size_t vbo_index = attrib.vao_vbo_bind_index;
            if(vbos_to_deduce_stride_of.contains(vbo_index)) {
                size_t attrib_size = attrib.size * gal::typeid_to_size(attrib.type_id);
                vbos[vbo_index].set_stride(vbos[vbo_index].get_stride() + attrib_size);
            }
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
        EXPECTS(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        // TODO: currently only supporting indexed data; if primitive.indices is undefined(-1) then the primitive is non-indexed
        EXPECTS(primitive.indices != -1);
        // TODO: currently the only types supported by gal::index_buffer are unsigned int and unsigned short; are there others we should be supporting?
        EXPECTS(indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
            || indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);

        // TODO: currently all mesh primitives must have distinct vbos, which may cause duplication of data in VRAM
        vector<gal::vertex_buffer> vbos;
        vector<gal::index_buffer> ibos;
        unordered_map<int, int> bufview_to_vbo_map;

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

        deduce_vbo_strides(vbos, layout);

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

        EXPECTS(image.bits == 8);

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

    static engine::collision_shape load_mesh_as_collision_shape(const tinygltf::Model& model, const tinygltf::Mesh& mesh, collision_layers_bitmask is_layers = 0, collision_layers_bitmask sees_layers = 0) {
        //multiple collision_shape primitives are currently unsupported
        EXPECTS(mesh.primitives.size() == 1);
        const tinygltf::Primitive& primitive = mesh.primitives[0];

        const tinygltf::Accessor& indices_accessor = model.accessors[primitive.indices];
        // TODO: support other modes other than TRIANGLES for collision_shape
        EXPECTS(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        // TODO: currently only supporting indexed data for collision_shape; if primitive.indices is undefined(-1) then the primitive is non-indexed
        EXPECTS(primitive.indices != -1);
        // TODO: currently the only type supported for indices for collision_shape is unsigned int
        EXPECTS(indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);

        int position_accessor_idx = -1;
        for (auto& [attrib_name, accessor_idx] : primitive.attributes) {
            //the only relevant attribute for collision_shape is the position
            if(attrib_name != "POSITION") continue;
            position_accessor_idx = accessor_idx;
            break;
        }

        //there must be a position accessor
        ASSERTS(position_accessor_idx != -1);
        const tinygltf::Accessor& position_accessor = model.accessors[position_accessor_idx];
        //only vec3 are supported for position since we only support 3d collision
        EXPECTS(position_accessor.type == 3);
        const tinygltf::BufferView& position_bufview = model.bufferViews[position_accessor.bufferView];
        const tinygltf::Buffer& position_buf = model.buffers[position_bufview.buffer];

        const void* verts_ptr = &position_buf.data[position_bufview.byteOffset];
        size_t number_of_verts = position_bufview.byteLength / position_bufview.byteStride;
        size_t verts_offset = position_accessor.byteOffset;
        size_t verts_stride = position_bufview.byteStride;


        auto& indices_bufview = model.bufferViews[indices_accessor.bufferView];
        EXPECTS(indices_bufview.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
        auto& indices_buf = model.buffers[indices_bufview.buffer];
        std::span<const glm::uvec3> indices_span((glm::uvec3*)&indices_buf.data[indices_bufview.byteOffset], indices_bufview.byteLength / sizeof(glm::uvec3));

        return engine::collision_shape::from_mesh(verts_ptr, number_of_verts, verts_offset, verts_stride, indices_span, is_layers, sees_layers);
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
        if(mesh.name.ends_with("-col")) {
            return get_rm().new_from(load_mesh_as_collision_shape(model, mesh));
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
            throw std::runtime_error(std::format("file {} is not a gltf file: extension is not .glb or .gltf (error at engine::load_nodetree_from_gltf)", filepath));

        const tinygltf::Model model = load_gltf_from_file(filepath, binary);

        noderef root(filepath);

        const tinygltf::Scene& scene = model.scenes.at(0);
        list<int> node_idx_queue;
        for (int node_idx : scene.nodes)
            root.add_child(load_node_subtree(model, node_idx));

        return engine::nodetree_blueprint(std::move(root), nonempty_nodetree_name);
    }
}
