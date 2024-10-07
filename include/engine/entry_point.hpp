#ifndef ENTRY_POINT_HPP
#define ENTRY_POINT_HPP
#include <engine/window/window.hpp>
#include <engine/scene/basic_scene.hpp>

namespace engine {
    //this is only a temporary entry point; the window creation parameters and the start scene will be fetched from the asset-like file which defines the root of the game
    using get_start_scene_t = std::function<rc<basic_scene>()>;
    void entry_point(glm::ivec2 wnd_res, const char* wnd_name, window::window_creation_hints wnd_hints, get_start_scene_t get_start_scene);
}

#endif // ENTRY_POINT_HPP
