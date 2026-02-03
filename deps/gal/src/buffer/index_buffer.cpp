#include <GAL/buffer/index_buffer.hpp>
#include <glad/glad.h>

using namespace glm;

namespace gal {
    index_buffer::index_buffer(buffer buf, size_t tri_count, uint element_typeid) : m_buf(std::move(buf)), m_triangle_count(tri_count), m_element_typeid(element_typeid) {}

    size_t index_buffer::stride() {
        return 3 * (
                   m_element_typeid == GL_UNSIGNED_INT ? sizeof(uint) :
                       m_element_typeid == GL_UNSIGNED_SHORT ? sizeof(ushort) : 0
                   );
    }

    index_buffer::index_buffer(index_buffer &&o) noexcept: index_buffer(std::move(o.m_buf), o.m_triangle_count, o.m_element_typeid) {}

    index_buffer::index_buffer(const uvec3* data, size_t size, buffer_creation_params params) : index_buffer(buffer(data, size*sizeof(uvec3), params), size, GL_UNSIGNED_INT) { }

    index_buffer::index_buffer(const u16vec3* data, size_t size, buffer_creation_params params) : index_buffer(buffer(data, size*sizeof(u16vec3), params), size, GL_UNSIGNED_SHORT) { }

    index_buffer::index_buffer(uint element_typeid, size_t size, buffer_creation_params params) : index_buffer(buffer(nullptr, size, params), size, element_typeid) { }

    uint index_buffer::get_gl_id() const { return m_buf.get_gl_id(); }

    uint index_buffer::get_element_typeid() const { return m_element_typeid; }

    uint index_buffer::get_triangle_count() const { return m_triangle_count; }
}
