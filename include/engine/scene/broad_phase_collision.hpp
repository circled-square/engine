#ifndef ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP
#define ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP

#include <vector>

#include <engine/resources_manager/rc.hpp>

namespace engine {
    class node_data;

    /* broad phase collision detector interface:
     * it is an abstract class even though the bpcd will never be "hot-swappable";
     * it will simply be useful for development and debugging.
     */
    class broad_phase_collision_detector {
    public:
        broad_phase_collision_detector() = default;
        virtual ~broad_phase_collision_detector() = default;

        virtual void check_collisions_and_trigger_reactions() = 0;
        virtual void subscribe(node_data&) = 0;
        virtual void reset_subscriptions() = 0;
    };

    /* pass all bpcd, a naïve implementation of bpcd:
     * simply passes everything to the narrow phase: not computationally viable, especially
     * for large scenes, but useful for debugging and as a first working implementation.
    `*/
    class pass_all_broad_phase_collision_detector : public broad_phase_collision_detector {
        std::vector<node_data*> m_subscribers;
    public:
        pass_all_broad_phase_collision_detector() = default;
        virtual ~pass_all_broad_phase_collision_detector() = default;

        virtual void check_collisions_and_trigger_reactions();
        virtual void subscribe(node_data& n);
        virtual void reset_subscriptions();
    };
}

#endif // ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP
