#ifndef GAL_BUFFER_HPP
#define GAL_BUFFER_HPP

#include <glad/glad.h>
#include <GAL/types.hpp>
#include <concepts>

namespace gal {
    struct buffer_creation_params {
        bool is_static = true;
        // apply when is_static = true => glNamedBufferStorage
        unsigned flags = GL_DYNAMIC_STORAGE_BIT;
        // apply when is_static = false => glNamedBufferData
        unsigned hints = GL_DYNAMIC_DRAW;
    };

    template<typename T>
    concept Array = requires (T arr) {
        {arr.data()} -> std::convertible_to<typename T::const_pointer>;
        {arr.size()} -> std::convertible_to<std::size_t>;
        {arr[size_t(0)]} -> std::convertible_to<typename T::const_reference>;
    };

    class buffer {
        unsigned m_buf_id;
    public:
        buffer(const void* data, std::size_t size, buffer_creation_params params = { });

        template<Array arr_t>
        buffer(const arr_t& arr, buffer_creation_params params = {})
            : buffer(arr.data(), arr.size() * sizeof(typename arr_t::value_type), params) {}

        buffer(buffer&& o)  noexcept;

        ~buffer();

        void update(std::size_t offset, const void *data, std::size_t size);


        bool is_null() const { return m_buf_id == 0; }
        unsigned get_gl_id() const { return m_buf_id; }
    };
}

#endif //GAL_BUFFER_HPP
