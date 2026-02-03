#include <GAL/renderer/renderer.hpp>
#include <GAL/types.hpp>
#include <glad/glad.h>

namespace gal {
    void renderer::draw(const vertex_array& vao, const shader_program& shader, unsigned ibo_index) {
        draw(vao, shader, 0, vao.get_triangle_count(ibo_index), ibo_index);
    }
    void renderer::draw(const vertex_array& vao, const shader_program& shader, size_t ibo_start, size_t ibo_count, unsigned ibo_index) {
        shader.bind();
        vao.bind(ibo_index);
        uint ibo_elem_typeid = vao.get_ibo_element_typeid(ibo_index);
        std::size_t ibo_elem_size = typeid_to_size(ibo_elem_typeid);

        glDrawElements(GL_TRIANGLES, ibo_count * 3, ibo_elem_typeid, (const void*)(ibo_elem_size * 3 * ibo_start)); //draw triangles({1}), read {2} elements of type {3} starting from the {4}th byte from the bound GL_ELEMENT_ARRAY_BUFFER
    }
    void renderer::draw_without_indices(const vertex_array& vao, const shader_program& shader, size_t first, size_t count) {
        shader.bind();
        vao.bind();

        glDrawArrays(GL_TRIANGLES, first, count);
    }


    void renderer::clear(glm::vec4 c) {
        glClearColor(c.x, c.y, c.z, c.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void renderer::change_viewport_size(glm::ivec2 s) {
        glViewport(0,0, s.x, s.y);
    }

    void renderer::set_render_flags(render_flags flags) {
        if(flags.perform_alpha_blend) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glEnable(GL_BLEND);
        }

        if(flags.depth_test == depth_test_t::keep_less) {
            glEnable(GL_DEPTH_TEST); // Enable depth test
            glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
        } else if(flags.depth_test == depth_test_t::keep_more) {
            glEnable(GL_DEPTH_TEST); // Enable depth test
            glDepthFunc(GL_LESS); // Accept fragment if it is father from the camera than the former one
        }

        if(flags.face_culling == face_culling_t::back) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else if(flags.face_culling == face_culling_t::front) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }
    }
}

