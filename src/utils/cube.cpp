#include <engine/utils/cube.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
namespace engine {
    namespace cube {
        using vertex_t = engine::retro_3d_shader_vertex_t;
        static constexpr std::array<vertex_t, 16> vertex_data {
            vertex_t
            { { .5,  .5,  .5}, {1., 1.} },
            { { .5,  .5, -.5}, {0., 1.} },
            { {-.5,  .5, -.5}, {1., 1.} },
            { {-.5,  .5,  .5}, {0., 1.} },

            { { .5, -.5,  .5}, {1., 0.} },
            { { .5, -.5, -.5}, {0., 0.} },
            { {-.5, -.5, -.5}, {1., 0.} },
            { {-.5, -.5,  .5}, {0., 0.} },

            //top verts with tex coords fixed for top face
            { { .5,  .5,  .5}, {1., 1.} },
            { { .5,  .5, -.5}, {1., 0.} },
            { {-.5,  .5, -.5}, {0., 0.} },
            { {-.5,  .5,  .5}, {0., 1.} },

            //bottom verts with tex coords fixed for bottom face
            { { .5, -.5,  .5}, {1., 1.} },
            { { .5, -.5, -.5}, {1., 0.} },
            { {-.5, -.5, -.5}, {0., 0.} },
            { {-.5, -.5,  .5}, {0., 1.} },
        };
        static constexpr std::array<glm::uvec3, 12> indices {
            glm::uvec3
            {8,  9,  10},
            {10, 11, 8 },

            {12, 15, 14},
            {14, 13, 12},

            {1, 0, 4},
            {4, 5, 1},

            {2, 1, 5},
            {5, 6, 2},

            {3, 2, 6},
            {6, 7, 3},

            {0, 3, 7},
            {7, 4, 0},
        };

        rc<const engine::collision_shape> make_col_shape(engine::collision_layers_bitmask is_layers, engine::collision_layers_bitmask sees_layers) {
            return get_rm().new_from(engine::collision_shape::from_mesh(
                engine::stride_span<const glm::vec3>(vertex_data.data(), offsetof(vertex_t, pos), sizeof(vertex_t), vertex_data.size()),
                std::span<const glm::uvec3>(indices.begin(), indices.end()),
                is_layers, sees_layers
            ));
        }
        gal::vertex_array make_vao() {
            return gal::vertex_array::make<vertex_t>(vertex_data, std::span(indices.data(), indices.size()));
        }
    }
}
