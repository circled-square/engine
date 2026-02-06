#include <engine/scene/renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/scene/node/camera.hpp>

namespace engine {
    void renderer::clear(glm::vec4 c) { m_low_level_renderer.clear(c); }

    void renderer::draw(const mesh& mesh, glm::ivec2 output_resolution, const mvp_matrices& mvp, float frame_time) {
        for(const primitive& prim : mesh.primitives()) {
            const material& material = prim.primitive_material;
            material.bind_and_set_uniforms(mvp, output_resolution, frame_time);
            //draws all triangles in ibo 0, from 0 to the last
            m_low_level_renderer.draw(*prim.vao, material.get_shader()->get_program(), 0, prim.vao->get_triangle_count(0), 0);
        }
    }

    gal::renderer &renderer::get_low_level_renderer() { return m_low_level_renderer; }
}
