#include <GAL/types.hpp>
#include <glad/glad.h>

namespace gal {
    // check for correctness of the defined types
    static_assert(sizeof(fixed) == sizeof(GLfixed) && alignof(fixed) == alignof(GLfixed));
    static_assert(sizeof(half) == sizeof(GLhalf) && alignof(half) == alignof(GLhalf));
    static_assert(sizeof(bool) == sizeof(GLboolean) && alignof(bool) == alignof(GLboolean));
    static_assert(std::same_as<sbyte, GLbyte>);
    static_assert(std::same_as<ubyte, GLubyte>);
    static_assert(std::same_as<sshort, GLshort>);
    static_assert(std::same_as<ushort, GLushort>);
    static_assert(std::same_as<sint, GLint>);
    static_assert(std::same_as<uint, GLuint>);
    static_assert(std::same_as<int64, GLint64>);
    static_assert(std::same_as<uint64, GLuint64>);

    // check correctness of OpenGL type IDs
    static_assert(gl_type_id<sbyte> == GL_BYTE);
    static_assert(gl_type_id<ubyte> == GL_UNSIGNED_BYTE);
    static_assert(gl_type_id<sshort> == GL_SHORT);
    static_assert(gl_type_id<ushort> == GL_UNSIGNED_SHORT);
    static_assert(gl_type_id<sint> == GL_INT);
    static_assert(gl_type_id<uint> == GL_UNSIGNED_INT);
    static_assert(gl_type_id<float> == GL_FLOAT);
    static_assert(gl_type_id<double> == GL_DOUBLE);
    static_assert(gl_type_id<half> == GL_HALF_FLOAT);
    static_assert(gl_type_id<fixed> == GL_FIXED);
    static_assert(gl_type_id<bool> == GL_BOOL);

    // quickly test glm matrix and vector concepts
    static_assert(!GlmVector<float> &&  GlmVector<glm::vec2> && !GlmVector<glm::mat3>, "GlmVector concept test failed!");
    static_assert(!GlmMatrix<float> && !GlmMatrix<glm::vec2> &&  GlmMatrix<glm::mat3>, "GlmMatrix concept test failed!");

}

