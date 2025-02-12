#include <engine/scene/renderer/mesh/material.hpp>

namespace engine {
    material::material(const material &o) : m_shader(o.m_shader), m_textures(o.m_textures) {}

    material::material(material &&o) : m_shader(std::move(o.m_shader)), m_textures(std::move(o.m_textures)) {}

    material::material(rc<const shader> shader, std::vector<rc<const gal::texture>> textures)
        : m_shader(std::move(shader)), m_textures(std::move(textures)) {
        EXPECTS(m_shader->get_uniforms().sampler_names.size() == m_textures.size());
    }

    material::material(rc<const shader> shader, rc<const gal::texture> texture) : material(std::move(shader), std::vector { std::move(texture) }){}

    const rc<const shader>& material::get_shader() const { return m_shader; }

    const std::vector<rc<const gal::texture>>& material::get_textures() const { return m_textures; }

    rc<const gal::texture>& material::get_texture(size_t index) {
        EXPECTS(index < m_textures.size());
        return m_textures[index];
    }
    const rc<const gal::texture>& material::get_texture(size_t index) const {
        EXPECTS(index < m_textures.size());
        return m_textures[index];
    }


    void material::bind_and_set_uniforms(glm::mat4 mvp, glm::ivec2 output_resolution, float frame_time) const {
        EXPECTS(m_shader->get_uniforms().sampler_names.size() == m_textures.size());
        m_shader->get_program().bind();

        if(m_shader->get_uniforms().output_resolution) {
            m_shader->get_program().set_uniform<glm::ivec2>(uniform_names::output_resolution, output_resolution);
        }

        if(m_shader->get_uniforms().time) {
            m_shader->get_program().set_uniform(uniform_names::time, frame_time);
        }

        if(m_shader->get_uniforms().mvp) {
            m_shader->get_program().set_uniform(uniform_names::mvp, mvp);
        }


        size_t next_texture_slot = 0;
        for(; next_texture_slot < m_textures.size(); next_texture_slot++) {
            const std::string& name = m_shader->get_uniforms().sampler_names[next_texture_slot].c_str();
            const gal::texture& texture = *m_textures[next_texture_slot];

            texture.bind(next_texture_slot);
            m_shader->get_program().set_uniform<int>(name.c_str(), next_texture_slot);
        }
    }

    material& material::operator=(material&& o) {
        m_shader = std::move(o.m_shader);
        m_textures = std::move(o.m_textures);

        return *this;
    }
}
