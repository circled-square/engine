#include <GAL/shader.hpp>
#include <GAL/types.hpp>
#include <stdexcept>
#include <format>
#include <vector>
#include <glad/glad.h>

namespace gal {
    uint shader_program::compile_shader(uint type, const std::string &source) {
        uint id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        sint compilation_result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compilation_result);
        if (!compilation_result) {
            sint length;
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

    shader_program::shader_program(const std::string &vert_shader, const std::string &frag_shader) {
        this->m_program_id = glCreateProgram();
        uint vs = compile_shader(GL_VERTEX_SHADER, vert_shader);
        uint fs = compile_shader(GL_FRAGMENT_SHADER, frag_shader);

        glAttachShader(m_program_id, vs);
        glAttachShader(m_program_id, fs);

        glLinkProgram(m_program_id);
        glValidateProgram(m_program_id);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    shader_program::shader_program(shader_program&& o) {
        m_program_id = o.m_program_id;
        o.m_program_id = 0;
        m_uniform_location_cache = std::move(o.m_uniform_location_cache);
    }

    shader_program::~shader_program() {
        glDeleteProgram(m_program_id);
    }

    void shader_program::bind() const {
        glUseProgram(m_program_id);
    }

    //returns -1 if it cannot retrieve the uniform's location (because it does not exist or was optimized out)
    sint shader_program::get_uniform_location(const std::string& name) const {
        if(auto it = m_uniform_location_cache.find(name); it != m_uniform_location_cache.end()) {
            return it->second;
        }

        //cache miss
        sint location = glGetUniformLocation(m_program_id, name.c_str());

        m_uniform_location_cache.insert({ name, location });

        return location;
    }

    namespace internal {
        using umf_t = uniform_mat_func_t;

        umf_t type_to_uniform_mat_func__struct<glm::mat<2,2,float>>::v() { return (umf_t)glProgramUniformMatrix2fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<2,3,float>>::v() { return (umf_t)glProgramUniformMatrix2x3fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<2,4,float>>::v() { return (umf_t)glProgramUniformMatrix2x4fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<3,2,float>>::v() { return (umf_t)glProgramUniformMatrix3x2fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<3,3,float>>::v() { return (umf_t)glProgramUniformMatrix3fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<3,4,float>>::v() { return (umf_t)glProgramUniformMatrix3x4fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<4,2,float>>::v() { return (umf_t)glProgramUniformMatrix4x2fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<4,3,float>>::v() { return (umf_t)glProgramUniformMatrix4x3fv; }
        umf_t type_to_uniform_mat_func__struct<glm::mat<4,4,float>>::v() { return (umf_t)glProgramUniformMatrix4fv; }
    }

    namespace internal {
        uniform_func_t gl_type_id_to_uniform_func(uint id, std::size_t vec_size) {
            return (
                    id == gl_type_id<float> ? make_array(glProgramUniform1fv, glProgramUniform2fv, glProgramUniform3fv, glProgramUniform4fv) :
                    id == gl_type_id<double> ? make_array(glProgramUniform1dv, glProgramUniform2dv, glProgramUniform3dv, glProgramUniform4dv) :
                    id == gl_type_id<sint> || id == gl_type_id<bool> ? make_array(glProgramUniform1iv, glProgramUniform2iv, glProgramUniform3iv, glProgramUniform4iv) :
                    id == gl_type_id<uint> ? make_array(glProgramUniform1uiv, glProgramUniform2uiv, glProgramUniform3uiv, glProgramUniform4uiv) :
                    make_array(nullptr, nullptr, nullptr, nullptr)
                )[vec_size - 1];
        }
    }
}
