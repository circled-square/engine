#ifndef ENGINE_APPLICATION_HPP
#define ENGINE_APPLICATION_HPP

#include <glm/glm.hpp>
#include <engine/resources_manager/rc.hpp>

#include "scene.hpp"
#include "application/event.hpp"
#include "application/window.hpp"

namespace engine {
    class application {
        window::window m_window;

        rc<scene> m_active_scene;
        glm::vec2 m_prev_mouse_cursor_pos;
        bool m_ignore_mouse_move_on_next_event;
        std::vector<event_variant_t> m_events_this_frame;
    public:

        application(application&&) = delete; // application cannot be moved: m_window's window handle points to it in its user pointer and scene::m_application points to it

        //pass negative values in the resolution for fullscreen
        application(glm::ivec2 res, const std::string& title, window::creation_hints hints = window::creation_hints::NO_HINTS);

        ~application();

        //needs to be separate from the constructor because a scene can only be constructed after gl (gal) and resources_manager are initialized, which happens in application's constructor
        void set_start_scene(rc<scene> new_scene);

        void run();
    };
}

#endif // ENGINE_APPLICATION_HPP
