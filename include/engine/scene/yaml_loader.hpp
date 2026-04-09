#ifndef YAML_LOADER_HPP
#define YAML_LOADER_HPP

#include <engine/utils/api_macro.hpp>
#include <engine/scene.hpp>

namespace engine {
    ENGINE_API scene load_scene_from_yaml(const char* filename);
}
#endif // YAML_LOADER_HPP
