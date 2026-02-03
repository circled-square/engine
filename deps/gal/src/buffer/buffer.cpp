#include <GAL/buffer/buffer.hpp>
#include <glad/glad.h>

namespace gal {
    buffer::buffer(const void *data, size_t size, buffer_creation_params params) {
        glCreateBuffers(1, &m_buf_id);

        if(params.is_static) {
            unsigned flags = 0;
            if (params.flags & buffer_creation_params::flags_t::dynamic_storage)
                flags |= GL_DYNAMIC_STORAGE_BIT;
            glNamedBufferStorage(m_buf_id, size, data, params.flags);
        } else {
            unsigned hints = 0;
            if(params.flags & buffer_creation_params::hints_t::dynamic_draw)
                hints |= GL_DYNAMIC_DRAW;
            glNamedBufferData(m_buf_id, size, data, params.hints);
        }

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
