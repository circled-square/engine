#include <engine/scene/renderer/mesh/material/materials.hpp>

namespace engine {
    shader make_retro_3d_shader() {
        const char *vert = "#version 440 core \n\
            layout(location = 0) in vec3 pos; \
            layout(location = 1) in vec2 tex_coord; \
            out vec2 v_tex_coord; \
            uniform mat4 u_mvp; \
                \
            void main() { \
                gl_Position = u_mvp * vec4(pos, 1); \
                v_tex_coord = tex_coord; \
        }";
            const char *frag = "#version 330 core \n\
            in vec2 v_tex_coord; \
            uniform sampler2D u_texture_slot; \
            out vec4 color; \
                \
            void main() { \
                color = texture(u_texture_slot, v_tex_coord); \
        }";

        gal::shader_program program(vert, frag);
        std::vector<std::string> sampler_names;
        sampler_names.push_back("u_texture_slot");
        uniforms_info uniforms { .mvp = true, .sampler_names = sampler_names };

        return shader(std::move(program), std::move(uniforms));
    }
}
