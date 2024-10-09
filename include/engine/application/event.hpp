#ifndef ENGINE_APPLICATION_EVENT_HPP
#define ENGINE_APPLICATION_EVENT_HPP

#include <variant>
#include <glm/glm.hpp>

namespace engine {
    struct key_event_t {
        int key, scancode, action, mods;
    };
    struct mouse_move_event_t {
        glm::vec2 pos, movement;
    };
    using event_variant_t = std::variant<key_event_t, mouse_move_event_t>;
}

#endif // ENGINE_APPLICATION_EVENT_HPP
