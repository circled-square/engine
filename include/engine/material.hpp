#ifndef ENGINE_MATERIAL_HPP
#define ENGINE_MATERIAL_HPP

#include <vector>
#include <string>
#include <GAL/texture.hpp>
#include <GAL/shader.hpp>
#include <GAL/vertex_array.hpp> // for static vertex layout for retro_3d_shader
#include "rc.hpp"

namespace engine {
    // each boolean field represents a uniform; the field texture_samplers is a dictionary where each pair (s, t) represents a texture t and the name s of the sampler uniform
    struct uniforms_info {
        bool output_resolution = false;
        bool time = false;
        bool mvp = false;
        std::vector<std::string> sampler_names = std::vector<std::string>();
    };

    class shader {
        gal::shader_program m_program;
        uniforms_info m_uniforms;
    public:
        shader(gal::shader_program program, uniforms_info uniforms);

        static shader from_file(const char* path);
        static shader from_source(const char* source);

        gal::shader_program& get_program();
        const gal::shader_program& get_program() const;
        uniforms_info& get_uniforms();
        const uniforms_info& get_uniforms() const;
    };

    class material {
        rc<const shader> m_shader;
        //must be the same size as m_shader.uniforms.sampler_names
        std::vector<rc<const gal::texture>> m_textures;
    public:
        material() = delete;
        material(material&& o);
        material(const material& o);
        material(rc<const shader> shader, std::vector<rc<const gal::texture>> textures);
        material(rc<const shader> shader, rc<const gal::texture> texture);

        const rc<const engine::shader>& get_shader() const;
        const std::vector<rc<const gal::texture>>& get_textures() const;
        rc<const gal::texture>& get_texture(size_t);
        const rc<const gal::texture>& get_texture(size_t) const;

        void bind_and_set_uniforms(glm::mat4 mvp, glm::ivec2 output_resolution, float frame_time) const;

        material& operator=(material&& o);
    };
    namespace uniform_names {
        extern const char* output_resolution; // = "u_output_resolution";
        extern const char* time; // = "u_time";
        extern const char* mvp; // = "u_mvp";
    }
}

#endif // ENGINE_MATERIAL_HPP
