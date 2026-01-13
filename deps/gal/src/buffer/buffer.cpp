#include <GAL/buffer/buffer.hpp>

namespace gal {
    buffer::buffer(const void *data, size_t size, buffer_creation_params params) {
        glCreateBuffers(1, &m_buf_id);

        if(params.is_static)
            glNamedBufferStorage(m_buf_id, size, data, params.flags);
        else
            glNamedBufferData(m_buf_id, size, data, params.hints);

    }
    buffer::buffer(buffer &&o) noexcept {
        m_buf_id = o.m_buf_id;
        o.m_buf_id = 0;
    }
    buffer::~buffer() {
        if(m_buf_id) {
            glDeleteBuffers(1, &m_buf_id);
            m_buf_id = 0;
        }
    }

    void buffer::update(size_t offset, const void *data, size_t size) {
        glNamedBufferSubData(m_buf_id, offset, size, data);
    }
}
