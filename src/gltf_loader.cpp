#include <engine/gltf_loader.hpp>
#include <engine/materials.hpp>
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
    static tinygltf::Model load_gltf_from_file(const char *path, bool binary) {
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
            throw runtime_error("Fatal error: Failed to parse glTF");

        return model;
    }

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

    static gal::vertex_array get_vao_from_mesh_primitive(const tinygltf::Model& model, int mesh_idx, int primitive_idx) {
        const tinygltf::Mesh& mesh = model.meshes[mesh_idx];
        const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];
        const tinygltf::Accessor& indices_accessor = model.accessors[primitive.indices];
        //TODO: support other modes other than TRIANGLES
        EXPECTS(primitive.mode == TINYGLTF_MODE_TRIANGLES);
        // TODO: currently only supporting indexed data; if primitive.indices is undefined(-1) then the primitive is non-indexed
        EXPECTS(primitive.indices != -1);
        // TODO: currently the only types supported by gal::index_buffer are unsigned int and unsigned short; are there others we should be supporting?
        EXPECTS(indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
            || indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);

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

                if (!bufview_to_vbo_map.contains(bufview_idx)) {
                    vbo_bind = vbos.size();
                    bufview_to_vbo_map[bufview_idx] = vbo_bind;
                    vbos.push_back(make_vbo_from_bufview(model, bufview_idx));
                }

                //TODO: support matrices: this currently only works for scalars and vec2/3/4
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

    static gal::texture get_texture_from_mesh_primitive(const tinygltf::Model& model, int mesh_idx, int primitive_idx) {
        int material_idx = model.meshes[mesh_idx].primitives[primitive_idx].material;
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

    static engine::mesh load_mesh(const tinygltf::Model& model, int mesh_idx) {
        vector<engine::primitive> primitives;
        for(size_t primitive_idx = 0; primitive_idx < model.meshes[mesh_idx].primitives.size(); primitive_idx++) {
            gal::vertex_array vao = get_vao_from_mesh_primitive(model, mesh_idx, primitive_idx);
            gal::texture texture = get_texture_from_mesh_primitive(model, mesh_idx, primitive_idx);

            primitives.emplace_back(
                material(get_rm().get_retro_3d_shader(), get_rm().new_from<gal::texture>(std::move(texture))),
                get_rm().new_from<gal::vertex_array>(std::move(vao))
            );
        }
        return engine::mesh(std::move(primitives));
    }

    static glm::mat4 get_node_transform(const tinygltf::Node& n) {
        using namespace glm;

        mat4 rotation_mat = n.rotation.empty() ? mat4(1) : mat4_cast((quat)make_quat(n.rotation.data()));
        mat4 scale_mat = n.scale.empty() ? mat4(1) : scale(mat4(1), (vec3)make_vec3(n.scale.data()));
        mat4 translation_mat = n.translation.empty() ? mat4(1) : translate(mat4(1), vec3(n.translation[0], n.translation[1], n.translation[2]));
        mat4 raw_mat = n.matrix.empty() ? mat4(1) : (mat4)make_mat4(n.matrix.data());

        return translation_mat * scale_mat * rotation_mat * raw_mat;
    }

    static engine::node load_node_subtree(const tinygltf::Model& model, int idx) {
        const tinygltf::Node& node = model.nodes[idx];

        std::optional<engine::mesh> mesh = node.mesh != -1 ? std::optional{load_mesh(model, node.mesh)} : std::nullopt;

        glm::mat4 transform = get_node_transform(node);
        engine::node root = mesh ?
            engine::node(node.name, std::move(*mesh), transform)
            : engine::node(node.name, null_node_data{}, transform);

        for(int child_idx : node.children) {
            root.add_child(load_node_subtree(model, child_idx));
        }

        return root;
    }

    engine::nodetree load_nodetree_from_gltf(const char* filepath, const char* nodetree_name) {
        nodetree_name = nodetree_name ? nodetree_name : filepath;
        bool binary = string_view(filepath).ends_with(".glb");

        if(!binary && !string_view(filepath).ends_with(".gltf"))
            throw std::runtime_error(std::format("file {} is not a gltf file: extension is not .glb or .gltf (error at engine::load_nodetree_from_gltf)", filepath));

        const tinygltf::Model model = load_gltf_from_file(filepath, binary);

        engine::node root(filepath);

        const tinygltf::Scene& scene = model.scenes.at(0);
        list<int> node_idx_queue;
        for (int node_idx : scene.nodes)
            root.add_child(load_node_subtree(model, node_idx));

        return engine::nodetree(std::move(root), nodetree_name);
    }
}
