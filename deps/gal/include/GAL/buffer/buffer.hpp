#ifndef GAL_BUFFER_HPP
#define GAL_BUFFER_HPP

#include <GAL/types.hpp>
#include <GAL/api_macro.hpp>
#include <concepts>

namespace gal {

    struct buffer_creation_params {
        struct flags_t { bool dynamic_storage; };
        struct hints_t { bool dynamic_draw; };

        bool is_static = true;
        // apply when is_static = true => glNamedBufferStorage
        flags_t flags { .dynamic_storage = true }; // GL_DYNAMIC_STORAGE_BIT;
        // apply when is_static = false => glNamedBufferData
        hints_t hints { .dynamic_draw = true}; // GL_DYNAMIC_DRAW
    };

    template<typename T>
    concept Array = requires (T arr) {
        { arr.data() } -> std::convertible_to<typename T::const_pointer>;
        { arr.size() } -> std::convertible_to<std::size_t>;
        { arr[size_t(0)] } -> std::convertible_to<typename T::const_reference>;
    };

    class buffer {
        uint m_buf_id;
    public:
        GAL_API buffer(const void* data, std::size_t size, buffer_creation_params params = { });

        template<Array arr_t>
        buffer(const arr_t& arr, buffer_creation_params params = {})
            : buffer(arr.data(), arr.size() * sizeof(typename arr_t::value_type), params) {}

        buffer() = delete;
        buffer(const buffer&) = delete;
        GAL_API buffer(buffer&& o)  noexcept;

        GAL_API ~buffer();

        buffer& operator=(buffer&&) = delete;
        buffer& operator=(const buffer&) = delete;

        GAL_API void update(std::size_t offset, const void *data, std::size_t size);


        bool is_null() const { return m_buf_id == 0; }
        uint get_gl_id() const { return m_buf_id; }
    };
}

#endif //GAL_BUFFER_HPP
