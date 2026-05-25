#ifndef ENGINE_SCENE_NODE_COLLISION_BEHAVIOUR_HPP
#define ENGINE_SCENE_NODE_COLLISION_BEHAVIOUR_HPP

namespace engine {
    // specifies how a node reacts to collisions
    struct node_collision_behaviour {
        bool moves_away_on_collision : 1 = false;
        bool passes_events_to_script : 1 = false;
        bool passes_events_to_father : 1 = true;
    };
}

#endif // ENGINE_SCENE_NODE_COLLISION_BEHAVIOUR_HPP
