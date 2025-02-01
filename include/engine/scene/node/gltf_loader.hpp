#ifndef ENGINE_GLTF_LOADER_HPP
#define ENGINE_GLTF_LOADER_HPP

#include <engine/scene.hpp>

namespace engine {
    nodetree_blueprint load_nodetree_from_gltf(const char* filepath, const char* node_name = nullptr);
}

#endif // ENGINE_GLTF_LOADER_HPP
