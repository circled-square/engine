#include <GAL/shader.hpp>
#include <GAL/types.hpp>
#include <stdexcept>
#include <format>
#include <vector>
#include <glad/glad.h>

namespace gal {
    uint shader_program::compile_shader(uint type, std::string_view source) {
        uint id = glCreateShader(type);
        { // scope for parameters of glShaderSource
            const char* src = source.data();
            const sint length = (sint)source.length();
            glShaderSource(id, 1, &src, &length);
        }
        glCompileShader(id);

        sint compilation_result = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compilation_result);
        if (compilation_result == 0) {
            sint length = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> gl_message(length);
            glGetShaderInfoLog(id, length, &length, gl_message.data());

            glDeleteShader(id);
            throw std::runtime_error(
                std::format(
                    "Failed to compile {} shader\n\t{}",
                    type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                    gl_message.data()
                )
            );

        }

        return id;
    }

    shader_program::shader_program(std::string_view vert_shader, std::string_view frag_shader) : m_program_id(glCreateProgram()) {
        uint vs = compile_shader(GL_VERTEX_SHADER, vert_shader);
        uint fs = compile_shader(GL_FRAGMENT_SHADER, frag_shader);

        glAttachShader(m_program_id, vs);
        glAttachShader(m_program_id, fs);

        glLinkProgram(m_program_id);
        glValidateProgram(m_program_id);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    shader_program::shader_program(shader_program&& o) noexcept : m_program_id(o.m_program_id), m_uniform_location_cache(std::move(o.m_uniform_location_cache)) {
        o.m_program_id = 0;
    }

    shader_program::~shader_program() {
        glDeleteProgram(m_program_id);
    }

    void shader_program::bind() const {
        glUseProgram(m_program_id);
    }

    //returns -1 if it cannot retrieve the uniform's location (because it does not exist or was optimized out)
    sint shader_program::get_uniform_location(const char* name) const {
        if(auto it = m_uniform_location_cache.find(name); it != m_uniform_location_cache.end()) {
            return it->second;
        }

        //cache miss
        sint location = glGetUniformLocation(m_program_id, name);

        m_uniform_location_cache.insert({ std::string(name), location });

        return location;
    }

    namespace internal {
        using umf_t = uniform_mat_func_t;

        umf_t type_to_uniform_mat_func__struct<glm::mat<2,2,float>>::v() { return glProgramUniformMatrix2fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<2,3,float>>::v() { return glProgramUniformMatrix2x3fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<2,4,float>>::v() { return glProgramUniformMatrix2x4fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<3,2,float>>::v() { return glProgramUniformMatrix3x2fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<3,3,float>>::v() { return glProgramUniformMatrix3fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<3,4,float>>::v() { return glProgramUniformMatrix3x4fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<4,2,float>>::v() { return glProgramUniformMatrix4x2fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<4,3,float>>::v() { return glProgramUniformMatrix4x3fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<4,4,float>>::v() { return glProgramUniformMatrix4fv; }
    }

    namespace internal {
        template<> GAL_API std::array<uniform_func_t<float>, 4> type_to_uniform_funcs<float>() { return { glProgramUniform1fv, glProgramUniform2fv, glProgramUniform3fv, glProgramUniform4fv }; }
        template<> GAL_API std::array<uniform_func_t<double>, 4> type_to_uniform_funcs<double>() { return { glProgramUniform1dv, glProgramUniform2dv, glProgramUniform3dv, glProgramUniform4dv }; }
        template<> GAL_API std::array<uniform_func_t<sint>, 4> type_to_uniform_funcs<sint>() { return { glProgramUniform1iv, glProgramUniform2iv, glProgramUniform3iv, glProgramUniform4iv }; }
        template<> GAL_API std::array<uniform_func_t<bool>, 4> type_to_uniform_funcs<bool>() { return type_to_uniform_funcs<sint>(); }
        template<> GAL_API std::array<uniform_func_t<uint>, 4> type_to_uniform_funcs<uint>() { return { glProgramUniform1uiv, glProgramUniform2uiv, glProgramUniform3uiv, glProgramUniform4uiv }; }
    }
}
