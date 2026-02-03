#include "slogga/log.hpp"
#include <engine/scene/renderer/mesh/material.hpp>
#include <engine/utils/format_glm.hpp>

namespace engine {
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

        // set common uniforms
        if(m_shader->get_uniforms().output_resolution) {
            m_shader->get_program().set_uniform<glm::ivec2>(uniform_names::output_resolution, output_resolution);
        }
        if(m_shader->get_uniforms().time) {
            m_shader->get_program().set_uniform(uniform_names::time, frame_time);
        }
        if(m_shader->get_uniforms().mvp) {
            m_shader->get_program().set_uniform(uniform_names::mvp, mvp);
        }


        // set sampler uniforms
        for(size_t tex_slot = 0; tex_slot < m_textures.size(); tex_slot++) {
            const std::string& name = m_shader->get_uniforms().sampler_names[tex_slot].c_str();
            const gal::texture& texture = *m_textures[tex_slot];

            texture.bind(tex_slot);
            m_shader->get_program().set_uniform<int>(name.c_str(), tex_slot);
        }

        // set custom uniforms
        for(const auto& [name, value_variant] : m_custom_uniforms) {
            match_variant(value_variant,
            [&]<typename T>(const T& v) {
                m_shader->get_program().set_uniform<T>(name.c_str(), v);
            });
        }
    }

    material& material::operator=(material&& o) {
        m_shader = std::move(o.m_shader);
        m_textures = std::move(o.m_textures);
        m_custom_uniforms = std::move(o.m_custom_uniforms);

        return *this;
    }
}
