#ifndef ENGINE_BASIC_SCENE_HPP
#define ENGINE_BASIC_SCENE_HPP

#include <glm/glm.hpp>
#include "../rc.hpp"

namespace engine {
    struct application_channel_t {
        struct to_app_t {
            bool wants_mouse_cursor_captured = false;
            rc<basic_scene> scene_to_change_to = rc<basic_scene>();
        } to_app;
        struct from_app_t {
            bool scene_is_active = false;
            bool mouse_cursor_is_captured = false;
            glm::ivec2 framebuffer_size = {0,0};
        } from_app;
    };

    class basic_scene {
        application_channel_t m_application_channel;
    public:
        basic_scene();
        basic_scene(basic_scene&& o);
        basic_scene(basic_scene& o) = delete;

        virtual ~basic_scene() = default;

        //used by inheriting classes and by engine::application
        application_channel_t& application_channel();

        virtual void update(float delta) {}
        virtual void render(float frame_time) {}

        virtual void freeze() {}
        virtual void reheat() {}

        virtual const char* get_name() const { return "unnamed basic scene"; }

        virtual void on_key_press(int key, int scancode, int action, int mods) {}
        virtual void on_mouse_move(glm::vec2 position, glm::vec2 movement) {}
    };
};

#endif //ENGINE_BASIC_SCENE_HPP
