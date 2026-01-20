#include "slogga/asserts.hpp"
#include <engine/scene/node/viewport.hpp>
#include <engine/scene/node/narrow_phase_collision.hpp>
#include <engine/scene/node.hpp>
#include <engine/resources_manager.hpp>

namespace engine {
    viewport::viewport(framebuffer fbo, std::optional<glm::vec2> dynamic_size_relative_to_output)
        : m_fbo(std::move(fbo)), m_dynamic_size_relative_to_output(dynamic_size_relative_to_output) {
            EXPECTS(m_fbo.get_texture());
        }

    viewport::viewport(glm::vec2 dynamic_size_relative_to_output)
        // a texture with 0 texels causes the fbo to throw a framebuffer_construction_exception
        : viewport(framebuffer(get_rm().new_mut_from(gal::texture::null())), dynamic_size_relative_to_output) {}

    inline viewport copy(const viewport &o) {
        rc<gal::texture> new_texture = get_rm().new_mut_from<gal::texture>(gal::texture::empty(o.fbo().resolution(), 4));
        return viewport(framebuffer(std::move(new_texture)), o.dynamic_size_relative_to_output());
    }

    viewport::viewport(const viewport &o)
        : viewport(copy(o)) {}

    framebuffer &viewport::fbo() { return m_fbo; }

    const framebuffer &viewport::fbo() const { return m_fbo; }

    std::optional<glm::vec2> viewport::dynamic_size_relative_to_output() const { return m_dynamic_size_relative_to_output; }

    void viewport::bind_draw() const { m_fbo.bind_draw(); }

    void viewport::set_active_camera(const std::optional<camera>& c) { m_active_camera = c; }

    const std::optional<camera>& viewport::get_active_camera() const { return m_active_camera; }

    void viewport::output_resolution_changed(glm::ivec2 output_resolution) const {
        if(!m_dynamic_size_relative_to_output)
            return;

        glm::vec2 new_resolution_f = (glm::vec2)output_resolution * (*m_dynamic_size_relative_to_output);
        glm::ivec2 new_resolution = {std::round(new_resolution_f.x), std::round(new_resolution_f.y)};
        if(new_resolution != m_fbo.resolution()) {
            // a texture with 0 texels causes the fbo to throw a framebuffer_construction_exception
            if (new_resolution.x > 0 && new_resolution.y > 0) {
                gal::texture::specification spec {
                    .res = new_resolution,
                    .enable_mipmaps = false,
                    .filter_method = gal::texture::filter_method::nearest,
                };
                m_fbo.link_and_replace_texture(gal::texture(spec));
            }
        }
    }

    void viewport::operator=(viewport &&o) {
        m_fbo = std::move(o.m_fbo);
        m_dynamic_size_relative_to_output = std::move(o.m_dynamic_size_relative_to_output);
        m_active_camera = std::move(o.m_active_camera);
    }
}
