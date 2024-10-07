#include <engine/renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {
    camera::camera(glm::mat4 view_mat) : m_view_mat(view_mat) { }

    glm::mat4 camera::get_view_mat() const { return m_view_mat; }

    void camera::set_view_mat(glm::mat4 view_mat) { m_view_mat = view_mat; }

    void renderer::clear(glm::vec4 c) { m_low_level_renderer.clear(c); }

    void renderer::draw(const mesh& mesh, glm::ivec2 output_resolution, glm::mat4 mvp, float frame_time) {
        for(const primitive& prim : mesh.primitives()) {
            const material& material = prim.primitive_material;
            material.bind_and_set_uniforms(mvp, output_resolution, frame_time);
            //draws all triangles in ibo 0, from 0 to the last
            m_low_level_renderer.draw(*prim.vao, material.get_shader()->get_program(), 0, prim.vao->get_triangle_count(0), 0);
        }
    }
    gal::renderer &renderer::get_low_level_renderer() { return m_low_level_renderer; }

}
