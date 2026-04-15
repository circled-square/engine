#ifndef GAL_SHADER_HPP
#define GAL_SHADER_HPP

#include <GAL/types.hpp>

#include <string>
#include <array>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GAL/api_macro.hpp>

namespace gal {
    namespace internal {
        template<AnyOf<float, double, sint, bool, uint> T>
        struct uniform_func__struct { using type = void(*)(uint, sint, sint, const T*); };
        template<> struct uniform_func__struct<bool> { using type = uniform_func__struct<sint>::type; };

        template<GlmVector V> using map_bvec_to_ivec = std::conditional_t<std::same_as<typename V::value_type, bool>, glm::vec<V::length(), sint>, V>;

        template<AnyOf<float, double, sint, bool, uint> T>
        using uniform_func_t = uniform_func__struct<T>::type;
        using uniform_mat_func_t = void(*)(uint, sint, sint, ubyte, const float*); // the ubyte param is actually a boolean, but to be able to cast the function pointer we use ubyte since that is how OpenGL defines booleans

        //specialization for scalars and vectors
        template<typename T>
        GAL_API std::array<uniform_func_t<T>, 4> type_to_uniform_funcs();

        template<typename T>
        constexpr uniform_func_t<typename scalar_to_vector<T>::value_type> type_to_uniform_func() {
            using vec_t = scalar_to_vector<T>;
            using val_t = vec_t::value_type;
            constexpr uint id = gl_type_id<val_t>;
            constexpr std::size_t vec_size = vec_t::length();

            // use std::get so the bound is checked at compile time
            return std::get<vec_size-1>(type_to_uniform_funcs<val_t>());
        }


        //specialization for matrices
        // NOTE: there is no UniformMatrix functions for matrices of any type except float (as of OpenGL 4.6), so we only need to template it based on size
        template<typename matrix_t>
        struct type_to_uniform_mat_func__struct {};

        template<> struct type_to_uniform_mat_func__struct<glm::mat<2,2,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<2,3,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<2,4,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<3,2,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<3,3,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<3,4,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<4,2,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<4,3,float>>{ GAL_API static uniform_mat_func_t v(); };
        template<> struct type_to_uniform_mat_func__struct<glm::mat<4,4,float>>{ GAL_API static uniform_mat_func_t v(); };

        template<typename T>
        uniform_mat_func_t type_to_uniform_mat_func() { return type_to_uniform_mat_func__struct<T>::v(); }
    }

    //wrapper for a shader program. none of the functions need the shader to be bound
    class shader_program {
        uint m_program_id;
        mutable std::unordered_map<std::string, sint> m_uniform_location_cache;
        static uint compile_shader(uint type, const std::string& source);
    public:
        shader_program(const shader_program&) = delete;
        shader_program& operator=(shader_program&&) = delete;
        shader_program& operator=(const shader_program&) = delete;

        GAL_API explicit shader_program(const std::string& vert_shader, const std::string& frag_shader);
        GAL_API shader_program(shader_program&& o) noexcept;
        GAL_API ~shader_program();

        GAL_API void bind() const; //sets the program as active
        GAL_API sint get_uniform_location(const std::string& name) const;
        //overload this as needed
        void set_uniform(const char* name, auto v) const {
            set_uniform(get_uniform_location(name), v);
        }

        uint get_id() { return m_program_id; } //NOLINT(readability-make-member-function-const)

        /// TODO: add support for array uniforms
        template<typename T>
        void set_uniform(sint uniform_location, T v) const {
            internal::map_bvec_to_ivec<scalar_to_vector<T>> vec(v); // ensure we can just call value_ptr to obtain a ptr to the value
            using namespace internal;
            auto uniform_func = type_to_uniform_func<T>();
            //void glProgramUniform4fv(GLunsigned program, GLint location, GLsizei count, const GLfloat *value); (example signature; note that count should always be 1 if the targeted uniform is not an array)
            uniform_func(m_program_id, uniform_location, 1, glm::value_ptr(vec));
        }
        template<GlmMatrix T>
        void set_uniform(sint uniform_location, T v) const {
            using namespace internal;
            uniform_mat_func_t uniform_func = type_to_uniform_mat_func<T>();
            uniform_func(m_program_id, uniform_location, 1, false, glm::value_ptr(v));
        }
    };
}

#endif //GAL_SHADER_HPP
