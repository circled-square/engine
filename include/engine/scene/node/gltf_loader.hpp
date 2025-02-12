#ifndef ENGINE_SCENE_NODE_GLTF_LOADER_HPP
#define ENGINE_SCENE_NODE_GLTF_LOADER_HPP

#include <engine/scene.hpp>

namespace engine {
    //if node_name is left empty the node will have the name of the filepath
    nodetree_blueprint load_nodetree_from_gltf(const std::string& filepath, const std::string& node_name = "");
}

#endif // ENGINE_SCENE_NODE_GLTF_LOADER_HPP
