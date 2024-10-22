#ifndef ENGINE_MATERIALS_HPP
#define ENGINE_MATERIALS_HPP

#include "material.hpp"

namespace engine {
    shader make_retro_3d_shader();

    //vertex type expected by the shader produced by make_retro_3d_shader
    struct retro_3d_shader_vertex_t {
        glm::vec3 pos;
        glm::vec2 tex_coord;

        using layout_t = decltype(gal::static_vertex_layout(pos, tex_coord));
    };
}

#endif // ENGINE_MATERIALS_HPP
