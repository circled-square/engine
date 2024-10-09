#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include "node.hpp"
#include "../rc.hpp"
#include <glm/glm.hpp>
#include "../application/event.hpp"

namespace engine {
    struct application_channel_t {
        struct to_app_t {
            bool wants_mouse_cursor_captured = false;
            rc<scene> scene_to_change_to = rc<scene>();
        } to_app;
        struct from_app_t {
            bool scene_is_active = false;
            bool mouse_cursor_is_captured = false;
            glm::ivec2 framebuffer_size = {0,0};
            float delta = 0.f;
            float frame_time = 0.f;
            std::span<const event_variant_t> events;
        } from_app;

        application_channel_t(const application_channel_t&) = delete;
        application_channel_t(application_channel_t&&) = default;
        application_channel_t() = default;
        application_channel_t(to_app_t to_app, from_app_t from_app);
    };

    class scene {
        node m_root;
        std::string m_name;
        engine::renderer m_renderer;
        //for post processing
        rc<const gal::vertex_array> m_whole_screen_vao;

    protected: // TODO: make private
        application_channel_t m_application_channel;
    public:
        scene() = delete;
        scene(std::string s, node root = node(""), application_channel_t::to_app_t to_app_chan = application_channel_t::to_app_t());
        scene(scene&& o);

        void render();
        void update();
        void freeze() {}

        void reheat();
        const std::string& get_name() const { return m_name; }

        //used by engine::application
        const application_channel_t::to_app_t& channel_to_app() const;
        rc<scene> get_and_reset_scene_to_change_to();
        application_channel_t::from_app_t& channel_from_app();

        node& get_root();
        node& get_node(std::string_view path);
    };
}

#endif // ENGINE_SCENE_HPP
