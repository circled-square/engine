#ifndef ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP
#define ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP

#include <vector>

#include <engine/resources_manager/rc.hpp>

namespace engine {
    class node;

    /* broad phase collision detector interface:
     * it is an abstract class even though the bpcd will never be "hot-swappable";
     * it will simply be useful for development and debugging.
     */
    class broad_phase_collision_detector {
    public:
        broad_phase_collision_detector() = default;
        broad_phase_collision_detector(const broad_phase_collision_detector&) = default;
        broad_phase_collision_detector(broad_phase_collision_detector&&) = default;
        broad_phase_collision_detector& operator=(const broad_phase_collision_detector&) = default;
        broad_phase_collision_detector& operator=(broad_phase_collision_detector&&) = default;
        virtual ~broad_phase_collision_detector() = default;

        virtual void check_collisions_and_trigger_reactions() = 0;
        virtual void subscribe(node*) = 0;
        virtual void reset_subscriptions() = 0;
    };

    /* pass all bpcd, a naïve implementation of bpcd:
     * simply passes everything to the narrow phase: not computationally viable, especially
     * for large scenes, but useful for debugging and as a first working implementation.
    `*/
    class pass_all_broad_phase_collision_detector : public broad_phase_collision_detector {
        std::vector<node*> m_subscribers;
    public:
        void check_collisions_and_trigger_reactions() override;
        void subscribe(node* n) override;
        void reset_subscriptions() override;
    };
}

#endif // ENGINE_SCENE_BROAD_PHASE_COLLISION_HPP
