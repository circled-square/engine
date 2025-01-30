#ifndef ENGINE_SCENE_NODE_NP_COLLISIONS_HPP
#define ENGINE_SCENE_NODE_NP_COLLISIONS_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <array>

#include <optional>
#include <stdexcept>
#include <slogga/asserts.hpp>
#include <engine/utils/stack_vector.hpp>

namespace engine {
    class node;
    using collision_layer_index = std::uint8_t;
    using collision_layer_mask = stack_vector<collision_layer_index, 15>;

    static_assert(sizeof(collision_layer_mask) == 16);

    //TODO: collision_shape currently is not a resource; it should be, since it is immutable, shared among many nodes and expensive to copy and compute
    struct collision_shape {
        std::vector<glm::vec3> verts;
        std::vector<glm::vec3> face_normals;
        std::vector<glm::vec3> edges;
        //the correctness of layers in a collision is NOT checked by collision_shape;
        collision_layer_index is_layer;
        collision_layer_mask sees_layers;

        static collision_shape from_mesh(const void* mesh_verts_ptr, size_t mesh_verts_size, ptrdiff_t offset, ptrdiff_t stride, std::span<const glm::uvec3> mesh_indices, collision_layer_index is_layer, collision_layer_mask sees_layers);
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

        bool is_shallow() const;
        //returns the minimum translation the first collider should undertake to not collide with the second
        glm::vec3 get_min_translation();
        operator bool() const;

        collision_result operator-() { return collision_result { .versor = -versor, .depth = depth }; }
    };

    collision_result check_collision(const collision_shape& a, glm::mat4 a_trans, const collision_shape& b, glm::mat4 b_trans);
}

#endif // ENGINE_SCENE_NODE_NP_COLLISIONS_HPP
