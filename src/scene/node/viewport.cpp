#include <engine/scene/node/viewport.hpp>
#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    viewport::viewport(framebuffer fbo, rc<const shader> postfx_shader, std::optional<glm::vec2> dynamic_size_relative_to_output)
        : m_fbo(std::move(fbo)), m_postfx_material(std::move(postfx_shader), m_fbo.get_texture()),
          m_dynamic_size_relative_to_output(dynamic_size_relative_to_output) {}

    viewport::viewport(rc<const shader> postfx_shader, glm::vec2 dynamic_size_relative_to_output)
        : viewport(framebuffer(rc<gal::texture>()), std::move(postfx_shader), dynamic_size_relative_to_output) {}

    viewport::viewport(viewport &&o)
        : m_fbo(std::move(o.m_fbo)), m_postfx_material(std::move(o.m_postfx_material)),
        m_dynamic_size_relative_to_output(o.m_dynamic_size_relative_to_output) {}

    inline viewport copy(const viewport &o) {
        rc<gal::texture> new_texture = get_rm().new_mut_from<gal::texture>(gal::texture::empty(o.fbo().resolution(), 4));
        return viewport(framebuffer(std::move(new_texture)), o.postfx_material().get_shader(), o.dynamic_size_relative_to_output());
    }

    viewport::viewport(const viewport &o)
        : viewport(copy(o)) {}

    framebuffer &viewport::fbo() { return m_fbo; }

    const framebuffer &viewport::fbo() const { return m_fbo; }

    material &viewport::postfx_material() { return m_postfx_material; }

    const material &viewport::postfx_material() const { return m_postfx_material; }

    std::optional<glm::vec2> viewport::dynamic_size_relative_to_output() const { return m_dynamic_size_relative_to_output; }

    void viewport::bind_draw() const { m_fbo.bind_draw(); }

    void viewport::set_active_camera(const camera *c) { m_active_camera = c; }

    const camera *viewport::get_active_camera() const { return m_active_camera; }

    void viewport::output_resolution_changed(glm::ivec2 output_resolution) const {
        if(!m_dynamic_size_relative_to_output)
            return;

        glm::ivec2 new_resolution = (glm::vec2)output_resolution * (*m_dynamic_size_relative_to_output);
        if(new_resolution != m_fbo.resolution()) {
            // a texture with 0 texels causes the fbo to throw a framebuffer_construction_exception
            if (new_resolution.x > 0 && new_resolution.y > 0) {
                rc<gal::texture> new_texture = get_rm().new_mut_from<gal::texture>(gal::texture::empty(new_resolution, 4));
                m_postfx_material.get_texture(0) = new_texture;
                m_fbo.link_texture(std::move(new_texture));
            }
        }
    }

    void viewport::operator=(viewport &&o) {
        m_fbo = std::move(o.m_fbo);
        m_postfx_material = std::move(o.m_postfx_material);
        m_dynamic_size_relative_to_output = std::move(o.m_dynamic_size_relative_to_output);
        m_active_camera = std::move(o.m_active_camera);
    }
}
