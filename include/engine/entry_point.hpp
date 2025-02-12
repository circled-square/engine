#ifndef ENGINE_ENTRY_POINT_HPP
#define ENGINE_ENTRY_POINT_HPP

#include <engine/application/window.hpp>
#include <engine/scene.hpp>
#include <engine/resources_manager/rc.hpp>

namespace engine {
    //this is only a temporary entry point; the window creation parameters and the start scene will be fetched from the asset-like file which defines the root of the game
    void entry_point(glm::ivec2 wnd_res, const std::string& wnd_name, window::creation_hints wnd_hints, std::function<rc<scene>()> get_start_scene);
}

#endif // ENGINE_ENTRY_POINT_HPP
