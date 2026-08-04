#ifndef ENGINE_UTILS_CUBE_HPP
#define ENGINE_UTILS_CUBE_HPP

#include <GAL/vertex_array.hpp>
#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    namespace cube {
        ENGINE_API rc<const engine::collision_shape> make_col_shape(engine::collision_layers_bitmask is_layers, engine::collision_layers_bitmask sees_layers);
        gal::vertex_array make_vao();
    }
}
#endif // ENGINE_UTILS_CUBE_HPP
