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
    class spatial_hashmap {
        // permits spatial hashing
        struct ivec3_hash {
            std::size_t operator()(const glm::ivec3& v) const noexcept {
                std::hash<int> h;
                return h(v.x) ^ (h(v.y) << 1) ^ (h(v.z) << 2); // or use boost::hash_combine
            }
        };
        struct spatial_slot_t {
            // colliders for each of the layers; pointers are used,
            // which means entries must be repopulated at every frame
            // OR we need to manually subscribe nodes when they are created,
            // unsubscribe them when they are deleted and also sub/unsub them
            // when they are moved in memory and when they are moved spatially
            //(I prefer this idea, it'd allow colliders to be temp disabled)
            std::unordered_multimap<collision_layer_index, node*> colliders;
        };

        std::unordered_map<glm::ivec3, spatial_slot_t, ivec3_hash> m_hashmap;

        //size of the sides of each cubic slot the space is divided in inside the spatial hashmap
        float slot_size;

        glm::ivec3 position_to_slot(glm::vec3 pos) {
            return glm::ivec3(pos / slot_size);
        }

        std::vector<node*> scratch_vector;

    public:
        spatial_hashmap() {
            scratch_vector.reserve(128);
        }
        // TODO: implement a marking system so collisions between the same 2 colliders are not checked twice.
        // this is, however, complex since collision "visibility" can easily be non-mutual between 2 colliders and as
        // such we cannot simply mark any node after we checked the layers that it sees in, but we must also do the
        // "reverse" check of checking which colliders can see in the layer that it is in. This hopefully allows us to
        // mark entire spatial slots so less marking and mark-checking is performed.
        void check_collisions_and_trigger_reactions() {
            //work on spatial slot a time
            for(auto& [slot_position, slot] : m_hashmap) {
                stack_vector<spatial_slot_t*, 26> nearby_slots; // here a (bounded) vector is used instead of an array because not necessarily all of the entries will exist

                //get all nearby slots
                for(int dx = -1; dx <= 1; dx++) {
                    for(int dy = -1; dy <= 1; dy++) {
                        for(int dz = -1; dz <= 1; dz++) {
                            auto find_iter = m_hashmap.find(slot_position + glm::ivec3(dx, dy, dz));
                            if(find_iter != m_hashmap.end()) {
                                nearby_slots.push_back(&find_iter->second);
                            }
                        }
                    }
                }

                //for each of the nodes in the currently focused slot
                for(auto& [_col_layer, col_src_p] : slot.colliders) {

                    EXPECTS(col_src_p->has<collision_shape>());
                    collision_shape& src_cs = col_src_p->get<collision_shape>();

                    // find all possible colliders
                    std::vector<node*>& possible_collisions = scratch_vector;
                    possible_collisions.clear();

                    // by iterating over the nearby slots
                    for(spatial_slot_t* nearby_slot : nearby_slots) {
                        // and in each slot look only in the layers the focused node sees
                        for(collision_layer_index layer : src_cs.sees_layers) {
                            auto eq_range = nearby_slot->colliders.equal_range(layer);
                            for(auto it = eq_range.first; it != eq_range.second; it++) {
                                if(col_src_p != it->second) //ignore self-collision
                                    possible_collisions.push_back(it->second);
                            }
                        }
                    }

                    for(node* p : possible_collisions) {
                        EXPECTS(p->has<collision_shape>());
                        // TODO: global transform should be precomputed, at least for collision_shapes
                        // this is necessary anyway to be able to place them in slots.
                        collision_result r = check_collision(
                            src_cs, col_src_p->compute_global_transform(),
                            p->get<collision_shape>(), p->compute_global_transform()
                        );

                        // TODO: uhhh what now...?
                        //col_src_p->react_to_collision(r, p);
                    }
                }
            }
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
