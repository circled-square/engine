#ifndef GAL_RENDERER_HPP
#define GAL_RENDERER_HPP

#include "../vertex_array.hpp"
#include "../shader.hpp"

namespace gal {
    class renderer {
    public:
        void draw(const vertex_array& vao, const shader_program& shader, uint ibo_index = 0);
        void draw(const vertex_array& vao, const shader_program& shader, size_t ibo_start, size_t ibo_count, uint ibo_index = 0);
        void draw_without_indices(const vertex_array& vao, const shader_program& shader, size_t first, size_t count);

        void clear(glm::vec4 c = {0,0,0,0});
    };


}


#endif //GAL_RENDERER_HPP
