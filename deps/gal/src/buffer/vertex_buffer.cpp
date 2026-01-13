#include <GAL/buffer/vertex_buffer.hpp>

namespace gal {

    vertex_buffer::vertex_buffer(buffer buf, size_t stride) : m_buf(std::move(buf)), m_stride(stride) {}

    vertex_buffer::vertex_buffer(vertex_buffer &&o) noexcept: vertex_buffer(std::move(o.m_buf), o.m_stride) {}

    vertex_buffer::vertex_buffer(const void *data, size_t size, size_t stride, buffer_creation_params params)
            : vertex_buffer(buffer(data, size, params), stride) {}

    vertex_buffer::vertex_buffer(size_t size, size_t stride, buffer_creation_params params) : vertex_buffer(
            nullptr, size, stride, params) {}

    uint vertex_buffer::get_gl_id() const { return m_buf.get_gl_id(); }

    size_t vertex_buffer::get_stride() const { return m_stride; }

    void vertex_buffer::set_stride(size_t stride) { m_stride = stride; }
}
