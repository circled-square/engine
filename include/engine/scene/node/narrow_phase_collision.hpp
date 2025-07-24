#ifndef ENGINE_SCENE_NODE_NARROW_PHASE_COLLISION_HPP
#define ENGINE_SCENE_NODE_NARROW_PHASE_COLLISION_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <climits> // CHAR_BIT

#include <slogga/asserts.hpp>
#include <engine/utils/stride_span.hpp>

namespace engine {
    class node;
    using collision_layers_bitmask = std::uint64_t;

    // layers go from 0 to 63
    inline collision_layers_bitmask collision_layer(int n) {
        static_assert(sizeof(collision_layers_bitmask) * CHAR_BIT == 64);
        EXPECTS(0 <= n && n < 64);

        return collision_layers_bitmask(1) << n;
    }

    //TODO: make collision_shape an abstract class from which others inherit: (mesh|ray|sphere)_collision_shape
    struct collision_shape {
        std::vector<glm::vec3> verts;
        std::vector<glm::vec3> face_normals;
        std::vector<glm::vec3> edges;
        //the correctness of layers in a collision is NOT checked by collision_shape;
        collision_layers_bitmask is_layers;
        collision_layers_bitmask sees_layers;

        static collision_shape from_mesh(stride_span<const glm::vec3> mesh_verts, std::span<const glm::uvec3> mesh_indices, collision_layers_bitmask is_layer, collision_layers_bitmask sees_layers);
        static collision_shape from_mesh(stride_span<const glm::vec3> mesh_verts, std::span<const glm::u16vec3> mesh_indices, collision_layers_bitmask is_layer, collision_layers_bitmask sees_layers);
    };

    struct collision_behaviour {
        bool moves_away_on_collision : 1 = false;
        bool passes_events_to_script : 1 = false;
        bool passes_events_to_father : 1 = false;
    };

    struct collision_result {
        glm::vec3 versor;
        float depth;

        static collision_result null();

        //assumes collision occurred; a shallow collision is a collision with 0 depth
        bool is_shallow() const;
        //returns the minimum translation the first collider should undertake to not collide with the second
        glm::vec3 get_min_translation() const;
        // true if any collision occurred, even if shallow
        operator bool() const;

        collision_result inverse() const { return collision_result { .versor = -versor, .depth = depth }; }
        collision_result operator-() const { return inverse(); }
    };

    collision_result check_collision(const collision_shape& a, glm::mat4 a_trans, const collision_shape& b, glm::mat4 b_trans);
}

#endif // ENGINE_SCENE_NODE_NARROW_PHASE_COLLISION_HPP
