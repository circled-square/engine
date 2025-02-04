#include <engine/scene/broad_phase_collision.hpp>
#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/scene/node.hpp>
#include <slogga/asserts.hpp>

namespace engine {
    void pass_all_broad_phase_collision_detector::check_collisions_and_trigger_reactions() {
        for(size_t i = 0; i < m_subscribers.size(); i++) {
            EXPECTS(m_subscribers[i]);
            node& a = *m_subscribers[i];
            EXPECTS(a.has<collision_shape>());

            for(size_t j = i + 1; j < m_subscribers.size(); j++) {
                EXPECTS(m_subscribers[j]);
                node& b = *m_subscribers[j];
                EXPECTS(b.has<collision_shape>());

                //TODO: check layer correctness
                const collision_shape& a_cs = a.get<collision_shape>();
                const collision_shape& b_cs = b.get<collision_shape>();
                bool a_sees_b = a_cs.sees_layers & b_cs.is_layers;
                bool b_sees_a = b_cs.sees_layers & a_cs.is_layers;
                if(!a_sees_b && !b_sees_a)
                    continue;

                // TODO: global transform should be precomputed, at least for collision_shapes
                // this is necessary anyway to be able to place them in slots.
                collision_result res = check_collision(a_cs, a.compute_global_transform(), b_cs, b.compute_global_transform());

                if(res) {
                    if(a_sees_b)
                        a.react_to_collision(res, b);
                    if(b_sees_a)
                        b.react_to_collision(-res, a);
                }
            }
        }
    }

    void pass_all_broad_phase_collision_detector::subscribe(node& n) {
        m_subscribers.push_back(&n);
    }

    void pass_all_broad_phase_collision_detector::reset_subscriptions() {
        m_subscribers.clear();
    }
}
