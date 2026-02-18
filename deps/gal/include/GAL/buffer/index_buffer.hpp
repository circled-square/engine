#ifndef GAL_INDEX_BUFFER_HPP
#define GAL_INDEX_BUFFER_HPP

#include "buffer.hpp"
#include <GAL/api_macro.hpp>
#include <glm/glm.hpp>


namespace gal {
    template<typename T, typename U>
    concept ArrayOf = Array<T> && (std::same_as<typename T::value_type, const U> || std::same_as<typename T::value_type, U>);

    class index_buffer {
        buffer m_buf;
        size_t m_triangle_count; // store the size of the buffer to be passed to opengl for draw calls  (unless only part of the ibo is read)
        uint m_element_typeid; // GL_UNSIGNED_INT or GL_UNSIGNED_SHORT

        size_t stride();
    public:
        GAL_API index_buffer(index_buffer&& o)  noexcept;
        GAL_API index_buffer(buffer buf, size_t tri_count, uint element_typeid);

        template<ArrayOf<glm::uvec3> arr_t>
        index_buffer(const arr_t& arr, buffer_creation_params params = {})
            : index_buffer(arr.data(), arr.size(), params) {}

        template<ArrayOf<glm::u16vec3> arr_t>
        index_buffer(const arr_t& arr, buffer_creation_params params = {})
            : index_buffer(arr.data(), arr.size(), params) {}

        GAL_API index_buffer(const glm::uvec3* data, size_t tri_count, buffer_creation_params params = {});
        GAL_API index_buffer(const glm::u16vec3* data, size_t tri_count, buffer_creation_params params = {});
        GAL_API index_buffer(uint element_typeid, size_t tri_count, buffer_creation_params params = {});

        template<ArrayOf<glm::uvec3> arr_t>
        void update(const arr_t& arr, size_t offset = 0) {
            assert(m_element_typeid == gl_type_id<uint>);
            m_buf.update(offset, arr.data(), arr.size() * stride());
        }

        template<ArrayOf<glm::u16vec3> arr_t>
        void update(const arr_t& arr, size_t offset = 0) {
            assert(m_element_typeid == gl_type_id<ushort>);
            m_buf.update(offset, arr.data(), arr.size() * stride());
        }

        GAL_API uint get_gl_id() const;
        GAL_API uint get_element_typeid() const;
        GAL_API uint get_triangle_count() const;
    };
}

#endif //GAL_INDEX_BUFFER_HPP
