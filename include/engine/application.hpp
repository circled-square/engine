#ifndef ENGINE_APPLICATION_HPP
#define ENGINE_APPLICATION_HPP

#include "window/window.hpp"
#include <glm/glm.hpp>
#include <engine/rc.hpp>

#include "scene/basic_scene.hpp"

namespace engine {
    class application {
        window::window m_window;

        rc<basic_scene> m_active_scene;
        glm::vec2 m_prev_mouse_cursor_pos;
        bool m_ignore_mouse_move_on_next_event;
    public:

        application(application&&) = delete; // application cannot be moved: m_window's window handle points to it in its user pointer and scene::m_application points to it

        //pass negative values in the resolution for fullscreen
        application(glm::ivec2 res, const std::string& title, window::window_creation_hints hints = window::window_creation_hints::NO_HINTS);

        ~application();

        rc<basic_scene> set_active_scene(rc<basic_scene> new_scene);

        void run();
    };
}

#endif // ENGINE_APPLICATION_HPP
