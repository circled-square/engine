#include <engine/material.hpp>
#include <string_view>
#include <cstring>
#include <cctype>
#include <fstream>

namespace engine {
    namespace uniform_names {
        const char* output_resolution = "u_output_resolution";
        const char* time = "u_time";
        const char* mvp = "u_mvp";
    }

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
            const char* name = m_shader->get_uniforms().sampler_names[next_texture_slot].c_str();
            const gal::texture& texture = *m_textures[next_texture_slot];

            texture.bind(next_texture_slot);
            m_shader->get_program().set_uniform<int>(name, next_texture_slot);
        }
    }

    material& material::operator=(material&& o) {
        m_shader = std::move(o.m_shader);
        m_textures = std::move(o.m_textures);

        return *this;
    }

    static uniforms_info parse_uniforms(const std::string& s) {
        auto eat_whitespace = [](std::string_view& str) {
            while(std::isspace(str[0])) {
                str = str.substr(1);
            }
        };
        auto alnum_or_underscore = [](char c) {
            return c == '_' || std::isalnum(c);
        };

        std::string_view sv = s;
        eat_whitespace(sv);
        std::vector<std::pair<std::string_view, std::string_view>> uniforms_strings;

        while(!sv.empty()) {
            //consume "uniform" keyword
            std::string_view uniform_str = "uniform";
            if(!sv.starts_with(uniform_str)) {
                size_t show_until = sv.find(';');
                if(show_until < 0) show_until = sv.length();
                throw std::runtime_error(
                    std::format("parsing '#uniforms' block: expected 'uniform', got '{}'", sv.substr(0, show_until))
                );
            }
            sv = sv.substr(uniform_str.length());

            if(!std::isspace(sv[0])) {
                throw std::runtime_error(std::format("parsing '#uniforms' block: expected whitespace after 'uniforms', got '{}'", sv[0]));
            }

            eat_whitespace(sv);

            //consume type
            size_t end_of_type = 0;
            while(alnum_or_underscore(sv[end_of_type]))
                end_of_type++;
            std::string_view type_str = sv.substr(0, end_of_type);
            sv = sv.substr(type_str.length());

            if(!std::isspace(sv[0])) {
                throw std::runtime_error(std::format("parsing '#uniforms' block: expected whitespace after typename '{}', got '{}'", type_str, sv[0]));
            }

            eat_whitespace(sv);

            //consume uniform name
            size_t end_of_name = 0;
            while(alnum_or_underscore(sv[end_of_name]))
                end_of_name++;
            std::string_view name_str = sv.substr(0, end_of_name);
            sv = sv.substr(name_str.length());

            eat_whitespace(sv);

            //consume ";" token
            if(!sv.starts_with(';')) {
                throw std::runtime_error(std::format("parsing '#uniforms' block: expected ';' after uniform name '{}' got '{}'", name_str, sv[0]));
            }
            sv = sv.substr(1);

            eat_whitespace(sv);

            uniforms_strings.push_back({type_str, name_str});
        }

        uniforms_info uniforms_info;

        for(auto& [type_str, name_str] : uniforms_strings) {
            if(name_str == engine::uniform_names::mvp) {
                uniforms_info.mvp = true;
            } else if(name_str == engine::uniform_names::output_resolution) {
                uniforms_info.output_resolution = true;
            } else if(name_str == engine::uniform_names::time) {
                uniforms_info.time = true;
            } else {
                 if(type_str == "sampler2D") {
                    uniforms_info.sampler_names.push_back(std::string(name_str));
                } else {
                    throw std::runtime_error(std::format("could not parse uniform: type='{}', name='{}'", type_str, name_str));
                }
            }
        }

        return uniforms_info;
    }

    shader::shader(gal::shader_program program, uniforms_info uniforms)
        : m_program(std::move(program)), m_uniforms(std::move(uniforms)) {
    }


    shader shader::from_file(const char* path) {
        auto read_file = [](const char* fname) {
            std::ifstream fi(fname);
            if (fi.fail())
                throw std::runtime_error(std::format("shader::from_file could not read file at path '{}'", fname));
            std::string contents((std::istreambuf_iterator<char>(fi)), std::istreambuf_iterator<char>());
            return contents;
        };
        std::string content = read_file(path);
        return shader::from_source(content.c_str());
    }

    shader shader::from_source(const char* source) {
        std::string_view content = source;
        const char* uniforms_str = "#uniforms";
        const char* vert_str = "#vertex";
        const char* frag_str = "#fragment";

        size_t uniforms_str_pos = content.find(uniforms_str);
        size_t vert_str_pos = content.find(vert_str);
        size_t frag_str_pos = content.find(frag_str);

        // TODO: throw custom exception instead of using ASSERTS
        ASSERTS(uniforms_str_pos != std::string::npos);
        ASSERTS(vert_str_pos != std::string::npos);
        ASSERTS(frag_str_pos != std::string::npos);
        ASSERTS(uniforms_str_pos < vert_str_pos);
        ASSERTS(vert_str_pos < frag_str_pos);

        size_t uniforms_start = uniforms_str_pos + std::strlen(uniforms_str);
        size_t uniforms_end = vert_str_pos;
        size_t vert_start = vert_str_pos + std::strlen(vert_str);
        size_t vert_end = frag_str_pos;
        size_t frag_start = frag_str_pos + std::strlen(frag_str);
        size_t frag_end = content.size();

        std::string uniforms = std::string(content.substr(uniforms_start, uniforms_end - uniforms_start));
        std::string vert = std::string(content.substr(vert_start, vert_end - vert_start));
        std::string frag = std::string(content.substr(frag_start, frag_end - frag_start));

        return shader(
            gal::shader_program(vert, frag),
            parse_uniforms(uniforms)
        );
    }

    gal::shader_program &shader::get_program() { return m_program; }

    const gal::shader_program& shader::get_program() const { return m_program; }

    const uniforms_info& shader::get_uniforms() const { return m_uniforms; }

    uniforms_info& shader::get_uniforms() { return m_uniforms; }
}
