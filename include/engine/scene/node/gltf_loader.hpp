#ifndef ENGINE_SCENE_NODE_GLTF_LOADER_HPP
#define ENGINE_SCENE_NODE_GLTF_LOADER_HPP

#include <engine/scene.hpp>

namespace engine {
    //if node_name is left empty the node will have the name of the filepath
    nodetree_blueprint load_nodetree_from_gltf(const std::string& filepath, rc<const shader> shader, const std::string& node_name = "");

    class gltf_load_error : public std::exception {
        std::string m_filepath, m_err;
        mutable std::string m_what;
    public:
        enum type { PARSE_ERROR, FILE_EXTENSION_ERROR };
    private:
        type m_type;
    public:
        gltf_load_error(type t, std::string path, std::string err = "") : m_type(t), m_filepath(std::move(path)), m_err(std::move(err)) {}
        virtual const char* what() const noexcept;
    };
}

#endif // ENGINE_SCENE_NODE_GLTF_LOADER_HPP
