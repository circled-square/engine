#ifndef ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP
#define ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP

#include <vector>

#include "../rc.hpp"
//TODO: delete this fwd decl
namespace ImGui {
    void Text(const char*, ...);
}

namespace engine {
    class node;

    class broad_phase_collision_detector {
    public:
        broad_phase_collision_detector() = default;
        virtual ~broad_phase_collision_detector() = default;

        virtual void check_collisions_and_trigger_reactions() = 0;
        virtual void subscribe(node&) = 0;
        virtual void reset_subscriptions() = 0;
    };



    class pass_all_broad_phase_collision_detector
        : public broad_phase_collision_detector {
        std::vector<node*> m_subscribers;
    public:
        pass_all_broad_phase_collision_detector() = default;
        virtual ~pass_all_broad_phase_collision_detector() = default;

        virtual void check_collisions_and_trigger_reactions() {
            for(size_t i = 0; i < m_subscribers.size(); i++) {
                EXPECTS(m_subscribers[i]);
                node& a = *m_subscribers[i];
                EXPECTS(a.has<collision_shape>());

                for(size_t j = i + 1; j < m_subscribers.size(); j++) {
                    EXPECTS(m_subscribers[j]);
                    node& b = *m_subscribers[j];
                    EXPECTS(b.has<collision_shape>());

                    //TODO: check layer correctness

                    // TODO: global transform should be precomputed, at least for collision_shapes
                    // this is necessary anyway to be able to place them in slots.
                    collision_result res = check_collision(
                        a.get<collision_shape>(), a.compute_global_transform(),
                        b.get<collision_shape>(), b.compute_global_transform()
                    );

                    ImGui::Text("Collision? %b", (bool)res);


                    if(res) {
                        a.react_to_collision(res, b);
                        b.react_to_collision(-res, a);
                    }
                }
            }
        };
        virtual void subscribe(node& n) {
            m_subscribers.push_back(&n);
        };
        virtual void reset_subscriptions() {
            m_subscribers.clear();
        }
    };

    /*
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

        std::vector<node*> m_scratch_vector;

    public:
        spatial_hashmap() {
            m_scratch_vector.reserve(128);
        }


        // TODO: implement a marking system so collisions between the same 2 colliders are not checked twice.
        // this is, however, complex since collision "visibility" can easily be non-mutual between 2 colliders and as
        // such we cannot simply mark any node after we checked the layers that it sees in, but we must also do the
        // "reverse" check of checking which colliders can see in the layer that it is in. This hopefully allows us to
        // mark entire spatial slots so less marking and mark-checking is performed.
        void check_collisions_and_trigger_reactions() {
            //work on a spatial slot at a time
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
                    std::vector<node*>& possible_collisions = m_scratch_vector;
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
                        collision_result res = check_collision(
                            src_cs, col_src_p->compute_global_transform(),
                            p->get<collision_shape>(), p->compute_global_transform()
                        );


                        if(res) {
                            ASSERTS(col_src_p && p);
                            notify_of_collision(*col_src_p, *p, res);
                        }
                    }
                }
            }
        }
    };
    */
}

#endif // ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP
