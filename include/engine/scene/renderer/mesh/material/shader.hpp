#ifndef ENGINE_SCENE_RENDERER_MESH_MATERIAL_SHADER_HPP
#define ENGINE_SCENE_RENDERER_MESH_MATERIAL_SHADER_HPP

#include <vector>
#include <string>
#include <GAL/shader.hpp>
#include <GAL/vertex_array.hpp> // for static vertex layout for retro_3d_shader

namespace engine {
    // each boolean field represents a uniform; the field texture_samplers is a dictionary where each pair (s, t) represents a texture t and the name s of the sampler uniform
    struct uniforms_info {
        bool output_resolution = false;
        bool time = false;
        bool mvp = false;
        bool model = false;
        bool view = false;
        bool projection = false;
        bool dither_texture = false;

        std::vector<std::string> sampler_names;
    };


    class shader {
        gal::shader_program m_program;
        uniforms_info m_uniforms;
    public:
        shader(gal::shader_program program, uniforms_info uniforms);

        static shader from_file(const std::string& path);
        static shader from_source(std::string_view source);

        gal::shader_program& get_program();
        const gal::shader_program& get_program() const;
        uniforms_info& get_uniforms();
        const uniforms_info& get_uniforms() const;
    };

    namespace uniform_names {
        extern const char* const output_resolution; // = "u_output_resolution";
        extern const char* const time; // = "u_time";
        extern const char* const mvp; // = "u_mvp";
        extern const char* const model; // = "u_model";
        extern const char* const view; // = "u_view";
        extern const char* const projection; // = "u_projection";

        extern const char* const dither_texture;// = "u_dither_texture";
    }
}

#endif // ENGINE_SCENE_RENDERER_MESH_MATERIAL_SHADER_HPP
