#ifndef ENGINE_UTILS_STRIDE_SPAN_HPP
#define ENGINE_UTILS_STRIDE_SPAN_HPP

#include <cstddef> // std::size_t
#include <slogga/asserts.hpp>
#include <engine/utils/meta.hpp>

namespace engine {
    namespace internal {
        template<typename T>
        inline T* ptr_from_base_offset_stride_and_index(const_if<std::is_const_v<T>, void>* base, std::size_t offset, std::size_t stride, std::size_t index) {
            auto* base_cptr = reinterpret_cast<const_if<std::is_const_v<T>, char>*>(base);
            return reinterpret_cast<T*>(base_cptr + offset + (stride * index));
        }
    }

    template<typename T>
    class stride_span {
        const_if<std::is_const_v<T>, void>* m_base;
        std::size_t m_offset;
        std::size_t m_stride;
        std::size_t m_size;

        using byte_t = const_if<std::is_const_v<T>, char>;
    public:
        template<bool is_const>
        class iterator {
            const_if<is_const, char>* m_p;
            std::size_t m_stride;
        public:
            iterator(const_if<is_const, char>* p, std::size_t stride) : m_p(p), m_stride(stride) {}
            void operator++() { m_p += m_stride; }
            const_if<is_const, T>& operator*() { return *reinterpret_cast<const_if<is_const, T>*>(m_p); }
            bool operator!=(iterator& o) { return m_p != o.m_p; }
        };

        stride_span(const_if<std::is_const_v<T>, void>* base, std::size_t offset, std::size_t stride, std::size_t size)
            : m_base(base), m_offset(offset), m_stride(stride), m_size(size) {}

        stride_span() = delete;

        T& operator[](std::size_t i) {
            EXPECTS(i < m_size);
            return *internal::ptr_from_base_offset_stride_and_index<T>(m_base, m_offset, m_stride, i);
        }
        const T& operator[](std::size_t i) const {
            EXPECTS(i < m_size);
            return *internal::ptr_from_base_offset_stride_and_index<const T>(m_base, m_offset, m_stride, i);
        }

        std::size_t size() const { return m_size; }

        auto begin() {
            return iterator<std::is_const_v<T>>(internal::ptr_from_base_offset_stride_and_index<byte_t>(m_base, m_offset, m_stride, 0), m_stride);
        }
        auto begin() const {
            return iterator<true>(internal::ptr_from_base_offset_stride_and_index<const char>(m_base, m_offset, m_stride, 0), m_stride);
        }
        auto end() {
            return iterator<std::is_const_v<T>>(internal::ptr_from_base_offset_stride_and_index<byte_t>(m_base, m_offset, m_stride, m_size), m_stride);
        }
        auto end() const {
            return iterator<true>(internal::ptr_from_base_offset_stride_and_index<const char>(m_base, m_offset, m_stride, m_size), m_stride);
        }

        operator stride_span<const T>() { return stride_span<const T>(m_base, m_offset, m_stride, m_size); }
    };
}

#endif // ENGINE_UTILS_STRIDE_SPAN_HPP
