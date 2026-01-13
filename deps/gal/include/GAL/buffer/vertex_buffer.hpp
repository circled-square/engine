#ifndef GAL_VERTEX_BUFFER_HPP
#define GAL_VERTEX_BUFFER_HPP

#include "buffer.hpp"

namespace gal {
    class vertex_buffer {
        buffer m_buf;
        size_t m_stride;
    public:
        vertex_buffer(vertex_buffer&& o)  noexcept;
        vertex_buffer(buffer buf, size_t stride);

        vertex_buffer(const void* data, size_t size, size_t stride, buffer_creation_params params = {});
        vertex_buffer(size_t size, size_t stride, buffer_creation_params params = {});

        template<Array arr_t>
        vertex_buffer(const arr_t& arr, buffer_creation_params params = {}) 
            : vertex_buffer(buffer(arr, params), sizeof(typename arr_t::value_type)) {}

        void update(std::size_t offset, const void *data, std::size_t size) {
            m_buf.update(offset, data, size);
        }

        uint get_gl_id() const;
        size_t get_stride() const;
        void set_stride(size_t stride);
    };
}

#endif //GAL_VERTEX_BUFFER_HPP
