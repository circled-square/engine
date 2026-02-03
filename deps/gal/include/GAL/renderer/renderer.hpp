#ifndef GAL_RENDERER_HPP
#define GAL_RENDERER_HPP

#include "../vertex_array.hpp"
#include "../shader.hpp"

namespace gal {
    enum class depth_test_t { disabled = 0, keep_less, keep_more };
    enum class face_culling_t { disabled = 0, front, back };

    struct render_flags {
        depth_test_t depth_test = depth_test_t::keep_less;
        bool perform_alpha_blend = true;
        face_culling_t face_culling = face_culling_t::back;
    };

    class renderer {
    public:
        void draw(const vertex_array& vao, const shader_program& shader, unsigned ibo_index = 0);
        void draw(const vertex_array& vao, const shader_program& shader, size_t ibo_start, size_t ibo_count, unsigned ibo_index = 0);
        void draw_without_indices(const vertex_array& vao, const shader_program& shader, size_t first, size_t count);

        void clear(glm::vec4 c = {0,0,0,0});
        void change_viewport_size(glm::ivec2 s);
        void set_render_flags(render_flags flags);
    };
}


#endif //GAL_RENDERER_HPP
