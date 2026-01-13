#ifndef GAL_INTERNAL_TYPES_HPP
#define GAL_INTERNAL_TYPES_HPP

#include <glad/glad.h>
#include <cassert>
#include <concepts>
#include <glm/glm.hpp>

namespace gal {
    static_assert(sizeof(unsigned) == sizeof(GLuint), "unsigned int and GLuint do not have the same size");

    //make a struct for each of the types that opengl defines as aliases of other types,
    //so they can be treated as distinct types and passed to templates
    namespace types {
        struct fixed_point { GLfixed v; };
        struct f16 { GLhalf v;};
    }

    template <typename T, typename...O>
    concept AnyOf = (std::same_as<T, O> || ...);

    namespace detail {
        template<AnyOf<bool, GLbyte, GLubyte, GLshort, GLushort, GLint, GLuint, float, double, types::f16, types::fixed_point> T>
        struct gl_type_id__struct {};
        template<> struct gl_type_id__struct<bool>               { static constexpr unsigned v = GL_BOOL; };
        template<> struct gl_type_id__struct<GLbyte>             { static constexpr unsigned v = GL_BYTE; };
        template<> struct gl_type_id__struct<GLubyte>            { static constexpr unsigned v = GL_UNSIGNED_BYTE; };
        template<> struct gl_type_id__struct<GLshort>            { static constexpr unsigned v = GL_SHORT; };
        template<> struct gl_type_id__struct<GLushort>           { static constexpr unsigned v = GL_UNSIGNED_SHORT; };
        template<> struct gl_type_id__struct<GLint>              { static constexpr unsigned v = GL_INT; };
        template<> struct gl_type_id__struct<GLuint>             { static constexpr unsigned v = GL_UNSIGNED_INT; };
        template<> struct gl_type_id__struct<float>              { static constexpr unsigned v = GL_FLOAT; };
        template<> struct gl_type_id__struct<double>             { static constexpr unsigned v = GL_DOUBLE; };
        template<> struct gl_type_id__struct<types::f16>         { static constexpr unsigned v = GL_HALF_FLOAT; };
        template<> struct gl_type_id__struct<types::fixed_point> { static constexpr unsigned v = GL_FIXED; };
    }
    template<AnyOf<bool, GLbyte, GLubyte, GLshort, GLushort, GLint, GLuint, float, double, types::f16, types::fixed_point> T>
    constexpr unsigned gl_type_id = detail::gl_type_id__struct<T>::v;

    constexpr std::size_t typeid_to_size(unsigned id) {
        std::size_t size =
            id == GL_BOOL           ? sizeof(bool) :
            id == GL_BYTE           ? sizeof(GLbyte) :
            id == GL_UNSIGNED_BYTE  ? sizeof(GLubyte) :
            id == GL_SHORT          ? sizeof(GLshort) :
            id == GL_UNSIGNED_SHORT ? sizeof(GLushort) :
            id == GL_INT            ? sizeof(GLint) :
            id == GL_UNSIGNED_INT   ? sizeof(GLuint) :
            id == GL_FLOAT          ? sizeof(float) :
            id == GL_DOUBLE         ? sizeof(double) :
            id == GL_HALF_FLOAT     ? sizeof(GLhalf) :
            id == GL_FIXED          ? sizeof(GLfixed) :
            0;
        assert(size != 0);

        return size;
    }

    template<typename T> concept GlmVector = requires {
        { T::length() } -> std::convertible_to<std::size_t>;
        { T::x } -> std::convertible_to<typename T::value_type>;
    };
    template<typename T> concept GlmMatrix = requires {
        typename T::transpose_type;
    };
    static_assert(!GlmVector<float> &&  GlmVector<glm::vec2> && !GlmVector<glm::mat3>, "GlmVector concept test failed!");
    static_assert(!GlmMatrix<float> && !GlmMatrix<glm::vec2> &&  GlmMatrix<glm::mat3>, "GlmMatrix concept test failed!");

    //scalar_to_vector<T> wraps any scalar type into its vec<1, T> form, but doesn't wrap T if it is a glm vector
    namespace detail {
        template<typename T>
        struct scalar_to_vector_helper {
            using type = glm::vec<1, T, glm::defaultp>;
        };
        template<GlmVector vec_t>
        struct scalar_to_vector_helper<vec_t>{
            using type = vec_t;
        };
    }
    
    template<typename T>
    using scalar_to_vector = detail::scalar_to_vector_helper<T>::type;

    //vector_to_scalar<T> transforms any vec<n, T> into T, and returns T if it is a scalar
    template<typename T>
    using vector_to_scalar = scalar_to_vector<T>::value_type;
}

#endif // GAL_INTERNAL_TYPES_HPP
