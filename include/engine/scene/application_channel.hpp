#ifndef ENGINE_SCENE_APPLICATION_CHANNEL_HPP
#define ENGINE_SCENE_APPLICATION_CHANNEL_HPP

#include <engine/resources_manager/rc.hpp>
#include <engine/application/event.hpp>
#include <engine/utils/api_macro.hpp>
#include <glm/glm.hpp>


struct ImGuiContext; //avoid including imgui.h (and linking imgui) just for this opaque symbol

namespace engine {
    class scene;
    class application;

    struct application_channel_t {
        struct to_app_t {
            bool wants_mouse_cursor_captured = false;
            rc<scene> scene_to_change_to = rc<scene>();
            glm::vec4 clear_color = {0,0,0,0};
        };
        struct from_app_t {
            bool scene_is_active = false;
            bool mouse_cursor_is_captured = false;
            glm::ivec2 framebuffer_size = {0,0};
            float delta = 0.f;
            float frame_time = 0.f;
            std::span<const event_variant_t> events;

            ENGINE_API ImGuiContext* get_current_imgui_context() const; // this is necessary because each module (dll/exe) needs to call SetCurrentContext
        };

    private:
        from_app_t m_from_app;
        to_app_t m_to_app;

        friend class application;
        from_app_t& from_app_mut() { return m_from_app; }
    public:
        const from_app_t& from_app() const { return m_from_app; }
        to_app_t& to_app() { return m_to_app; }

        application_channel_t(const application_channel_t&) = delete;
        application_channel_t(application_channel_t&&) = default;
        application_channel_t() = default;
        ENGINE_API application_channel_t(to_app_t to_app, from_app_t from_app);
    };


}

#endif // ENGINE_SCENE_APPLICATION_CHANNEL_HPP
