#include <GAL/buffer/buffer.hpp>
#include <glad/glad.h>

namespace gal {
    buffer::buffer(const void *data, size_t size, buffer_creation_params params) : m_buf_id(0) {
        glCreateBuffers(1, &m_buf_id);

        if(params.is_static) {
            unsigned flags = 0;
            if (params.flags.dynamic_storage)
                flags |= GL_DYNAMIC_STORAGE_BIT;
            glNamedBufferStorage(m_buf_id, (int64)size, data, flags);
        } else {
            unsigned hints = 0;
            if(params.hints.dynamic_draw)
                hints |= GL_DYNAMIC_DRAW;
            glNamedBufferData(m_buf_id, (int64)size, data, hints);
        }

    }
    buffer::buffer(buffer &&o) noexcept : m_buf_id(o.m_buf_id) {
        o.m_buf_id = 0;
    }
    buffer::~buffer() {
        if(m_buf_id != 0) {
            glDeleteBuffers(1, &m_buf_id);
            m_buf_id = 0;
        }
    }

    // NOLINTBEGIN(readability-make-member-function-const)
    void buffer::update(size_t offset, const void *data, size_t size) {
        glNamedBufferSubData(m_buf_id, (int64)offset, (int64)size, data);
    }
    // NOLINTEND(readability-make-member-function-const)
}
