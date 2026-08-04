#ifndef YAML_LOADER_HPP
#define YAML_LOADER_HPP

#include <engine/utils/api_macro.hpp>
#include <engine/scene.hpp>

namespace engine {
    ENGINE_API scene load_scene_from_yaml(const char* filename);

    struct project_info_t {
        std::string window_name;
        std::optional<slogga::log_level> log_level;
        std::optional<glm::ivec2> resolution;
        bool maximised;
        std::string start_scene_path;
    };
    ENGINE_API project_info_t load_yaml_project_file(const char* filename);
}
#endif // YAML_LOADER_HPP
