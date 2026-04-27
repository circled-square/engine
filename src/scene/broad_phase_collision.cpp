#include <engine/scene/broad_phase_collision.hpp>
#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/scene/node.hpp>
#include <slogga/asserts.hpp>

namespace engine {
    inline node* assert_nonnull(node* p) {
        ASSERTS(p != nullptr);
        return p;
    }

    void pass_all_broad_phase_collision_detector::check_collisions_and_trigger_reactions() {
        for(size_t i = 0; i < m_subscribers.size(); i++) {
            node* a = assert_nonnull(m_subscribers[i]); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // i < m_subscribers.size()
            EXPECTS(a->has<rc<const collision_shape>>());

            for(size_t j = i + 1; j < m_subscribers.size(); j++) {
                node* b = assert_nonnull(m_subscribers[j]); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // i < m_subscribers.size()
                EXPECTS(b->has<rc<const collision_shape>>());

                //TODO: check layer correctness
                const auto& a_cs = a->get<collision_shape>();
                const auto& b_cs = b->get<collision_shape>();
                bool a_sees_b = bool(a_cs.sees_layers & b_cs.is_layers);
                bool b_sees_a = bool(b_cs.sees_layers & a_cs.is_layers);
                if(!a_sees_b && !b_sees_a)
                    continue;

                collision_result res = check_collision(a_cs, a->get_global_transform(), b_cs, b->get_global_transform());

                if(res) {
                    if(a_sees_b)
                        a->react_to_collision(res, *b);
                    if(b_sees_a)
                        b->react_to_collision(-res, *a);
                }
            }
        }
    }

    void pass_all_broad_phase_collision_detector::subscribe(node* n) {
        m_subscribers.push_back(n);
    }

    void pass_all_broad_phase_collision_detector::reset_subscriptions() {
        m_subscribers.clear();
    }
}
