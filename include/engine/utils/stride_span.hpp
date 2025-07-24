#ifndef ENGINE_UTILS_STRIDE_SPAN_HPP
#define ENGINE_UTILS_STRIDE_SPAN_HPP

#include <cstddef> // std::size_t
#include <slogga/asserts.hpp>

namespace engine {
    template<typename T>
    class stride_span {
        char* m_base;
        std::size_t m_offset;
        std::size_t m_stride;
        std::size_t m_size;

    public:
        class const_iterator {
            const char* m_p;
            std::size_t m_stride;
        public:
            const_iterator(const char* p, std::size_t stride) : m_p(p), m_stride(stride){ }
            void operator++() { m_p += m_stride; }
            const T& operator*() { return *reinterptet_cast<const T*>(m_p); }
            bool operator!=(const const_iterator& o) { return m_p != o.m_p; }
        };
        class iterator {
            char* m_p;
            std::size_t m_stride;
        public:
            iterator(char* p) : m_p(p){ }
            void operator++() { m_p += m_stride; }
            T& operator*() { return *reinterpret_cast<T*>(m_p); }
            bool operator!=(iterator& o) { return m_p != o.m_p; }
            operator const_iterator() { return const_iterator(m_p, m_stride); }
        };

        stride_span() = delete;
        stride_span(void* base, std::size_t offset, std::size_t stride, std::size_t size)
            : m_base(reinterpret_cast<char*>(base)), m_offset(offset), m_stride(stride), m_size(size) {}
        stride_span(const stride_span& o)
            : m_base(o.m_base), m_offset(o.m_offset), m_stride(o.m_stride), m_size(o.m_size) {}

        T& operator[](std::size_t i) {
            EXPECTS(i < m_size);
            return *reinterpret_cast<T*>(
                m_base + m_offset + m_stride * i
            );
        }
        const T& operator[](std::size_t i) const {
            EXPECTS(i < m_size);
            return *reinterpret_cast<const T*>(
                m_base + m_offset + m_stride * i
            );
        }

        std::size_t size() const { return m_size; }

        iterator begin() { return iterator(m_base+m_offset, m_stride); }
        const_iterator begin() const { return const_iterator(m_base+m_offset, m_stride); }
        iterator end() { return iterator(m_base + m_offset + m_stride * m_size, m_stride); }
        const_iterator end() const { return const_iterator(m_base + m_offset + m_stride * m_size, m_stride); }

        operator stride_span<const T>() { return stride_span<const T>(m_base, m_offset, m_stride, m_size); }
    };

    template<typename T>
    class stride_span<const T> {
        const char* m_base;
        std::size_t m_offset;
        std::size_t m_stride;
        std::size_t m_size;

    public:
        stride_span(const void* base, std::size_t offset, std::size_t stride, std::size_t size)
            : m_base(reinterpret_cast<const char*>(base)), m_offset(offset), m_stride(stride), m_size(size) {}
        stride_span(const stride_span& o)
            : m_base(o.m_base), m_offset(o.m_offset), m_stride(o.m_stride), m_size(o.m_size) {}

        const T& operator[](std::size_t i) const {
            EXPECTS(i < m_size);
            return *reinterpret_cast<const T*>(
                m_base + m_offset + m_stride * i
            );
        }

        std::size_t size() const { return m_size; }

        typename stride_span<T>::const_iterator begin() const { return stride_span<T>::const_iterator(m_base+m_offset, m_stride); }
        typename stride_span<T>::const_iterator end() const { return stride_span<T>::const_iterator(m_base + m_offset + m_stride * m_size, m_stride); }
    };
}

#endif // ENGINE_UTILS_STRIDE_SPAN_HPP
