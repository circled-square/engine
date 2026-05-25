#ifndef ENGINE_SCENE_NODE_SPAN_HPP
#define ENGINE_SCENE_NODE_SPAN_HPP

#include <span>
#include <slogga/asserts.hpp>

namespace engine {
    template<typename old_type, typename T>
    // requires requires(old_type o) { {T(o)} -> std::same_as<T>; }
    class map_span {
        std::span<old_type> m_span;

    public:
        class iterator {
            using span_iterator = std::span<old_type>::iterator;
            span_iterator m_it;
        public:
            iterator(span_iterator it) : m_it(it) {}
            void operator++() { m_it++; }
            bool operator!=(const iterator& o) const { return m_it != o.m_it; }
            T operator*() { return T(*m_it); }
        };

        map_span(std::span<old_type> span) : m_span(span) {}
        iterator begin() { return iterator(m_span.begin()); }
        iterator end() { return iterator(m_span.end()); }
        size_t size() const { return m_span.size(); }
        bool empty() const { return m_span.empty(); }
        T operator[](size_t i) const { EXPECTS(i < m_span.size()); return T(m_span[i]); } // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    };
}

#endif // ENGINE_SCENE_NODE_SPAN_HPP
