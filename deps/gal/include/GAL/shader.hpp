#ifndef GAL_SHADER_HPP
#define GAL_SHADER_HPP

#include <GAL/types.hpp>

#include <string>
#include <array>
#include <unordered_map>
#include <glm/glm.hpp>

namespace gal {
    namespace internal {
        using uniform_func_t = void(*)(uint, sint, sint, void*);
        using uniform_mat_func_t = void(*)(uint, sint, sint, ubyte, void*); // the ubyte param is actually a boolean, but to be able to cast the function pointer we use ubyte since that is how OpenGL defines booleans

        //given a list of gl uniform functions return an array of uniform_func_t
        template<typename... Ts>
        static constexpr std::array<uniform_func_t, sizeof...(Ts)> make_array(Ts... args){
            return { (uniform_func_t)args... };
        };

        //specialization for scalars and vectors
        uniform_func_t gl_type_id_to_uniform_func(uint id, std::size_t vec_size);

        template<typename T>
        uniform_func_t type_to_uniform_func() {
            using vec_t = scalar_to_vector<T>;
            using val_t = vec_t::value_type;
            static constexpr uint id = gl_type_id<val_t>;
            static constexpr std::size_t vec_size = vec_t::length();

            return gl_type_id_to_uniform_func(id, vec_size);
        }


        //specialization for matrices
        // NOTE: there is no UniformMatrix functions for matrices of any type except float (as of OpenGL 4.6), so we only need to template it based on size
        template<typename matrix_t>
        struct type_to_uniform_mat_func__struct {};

        template<> struct type_to_uniform_mat_func__struct<glm::mat<2,2,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<2,3,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<2,4,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<3,2,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<3,3,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<3,4,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<4,2,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<4,3,float>>{ static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<4,4,float>>{ static uniform_mat_func_t v(); };

        template<typename T>
        uniform_mat_func_t type_to_uniform_mat_func() { return type_to_uniform_mat_func__struct<T>::v(); }
    }

    //wrapper for a shader program. none of the functions need the shader to be bound
    class shader_program {
        uint m_program_id;
        mutable std::unordered_map<std::string, sint> m_uniform_location_cache;
        static uint compile_shader(uint type, const std::string& source);
    public:
        shader_program(const std::string& vert_shader, const std::string& frag_shader);
        shader_program(shader_program&& o);
        ~shader_program();

        void bind() const; //sets the program as active
        sint get_uniform_location(const std::string& name) const;
        //overload this as needed
        void set_uniform(const char* name, auto v) const {
            set_uniform(get_uniform_location(name), v);
        }

        uint get_id() { return m_program_id; }

        /// TODO: add support for array uniforms
        template<typename T>
        void set_uniform(sint uniform_location, T v) const {
            using namespace internal;
            uniform_func_t uniform_func = type_to_uniform_func<T>();
            //void glProgramUniform4fv(GLunsigned program, GLint location, GLsizei count, const GLfloat *value); (example signature; note that count should always be 1 if the targeted uniform is not an array)
            uniform_func(m_program_id, uniform_location, 1, &v);
        }
        template<GlmMatrix T>
        void set_uniform(sint uniform_location, T v) const {
            using namespace internal;
            uniform_mat_func_t uniform_func = type_to_uniform_mat_func<T>();
            uniform_func(m_program_id, uniform_location, 1, false, &v);
        }
    };
}

#endif //GAL_SHADER_HPP
