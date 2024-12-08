#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include <bit>

#include <glm/glm.hpp>

#include "node.hpp"
#include "../rc.hpp"
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

    namespace detail {
        struct ivec4_hash {
            std::size_t operator()(const glm::ivec4& v) const noexcept {
                std::size_t h1 = std::hash<unsigned>{}(v.x);
                std::size_t h2 = std::hash<unsigned>{}(v.y);
                std::size_t h3 = std::hash<unsigned>{}(v.z);
                std::size_t h4 = std::hash<unsigned>{}(v.z);
                return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h3 << 3); // or use boost::hash_combine
            }
        };
    }

    class spatial_hashmap {
        struct spatial_hashmap_entry {
            //colliders for each of the layers
            std::vector<node*> colliders;
        };

        //the w component of the ivec4 is for the layer of the collider
        std::unordered_map<glm::ivec4, spatial_hashmap_entry, detail::ivec4_hash> m_hashmap;

        //size of the sides of each cubic slot the space is divided in inside the spatial hashmap
        float slot_size;

        glm::ivec3 position_to_slot(glm::vec3 pos) {
            return glm::ivec3(pos / slot_size);
        }

    public:
        void check_collisions(node& n) {

            throw;
        }
    };

    class scene {
        node m_root;
        std::string m_name;
        engine::renderer m_renderer;
        //for post processing
        rc<const gal::vertex_array> m_whole_screen_vao;

        application_channel_t m_application_channel;

    public:
        scene() = delete;
        scene(std::string s, node root = node(""), application_channel_t::to_app_t to_app_chan = application_channel_t::to_app_t());
        scene(scene&& o);

        void prepare();

        void render();
        void update();

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
