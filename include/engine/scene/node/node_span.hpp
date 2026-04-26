#ifndef ENGINE_SCENE_NODE_SPAN_HPP
#define ENGINE_SCENE_NODE_SPAN_HPP

#include <span>
#include <memory>
#include <slogga/asserts.hpp>

namespace engine {
    class node;

    // const_node_span is used for iterating on the children of a node in an immutable manner.
    // we don't want to use std::span<std::unique_ptr<const node>> because the user would have to see (and could possibly create problems with) the std::unique_ptr
    // even if we wanted we cannot construct a span<ptr<const>> over a vector<ptr<mut>>
    class const_node_span {
        std::span<const std::unique_ptr<node>> m_span;
    public:
        class iterator {
            using span_iterator = std::span<const std::unique_ptr<node>>::iterator;
            span_iterator m_it;
        public:
            iterator(span_iterator it) : m_it(it) {}
            void operator++() { m_it++; }
            bool operator!=(const iterator& o) const { return m_it != o.m_it; }
            const node& operator*() { return **m_it; }
        };

        const_node_span(std::span<const std::unique_ptr<node>> span) : m_span(span) {}
        iterator begin() { return iterator(m_span.begin()); }
        iterator end() { return iterator(m_span.end()); }
        size_t size() const { return m_span.size(); }
        bool empty() const { return m_span.empty(); }
        const node& operator[](size_t i) const { EXPECTS(i < m_span.size()); return *m_span[i]; } // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    };


    // node_span is used for iterating on the children of a node in a mutable manner.
    // we don't want to use std::span<std::unique_ptr<node>> because the user would have to see (and could possibly create problems with) the std::unique_ptr
    class node_span {
        std::span<std::unique_ptr<node>> m_span;
    public:
        class iterator {
            using span_iterator = std::span<std::unique_ptr<node>>::iterator;
            span_iterator m_it;
        public:
            iterator(span_iterator it) : m_it(it) {}
            void operator++() { m_it++; }
            bool operator!=(const iterator& o) const { return m_it != o.m_it; }
            node& operator*() { return **m_it; }
        };

        node_span(std::span<std::unique_ptr<node>> span) : m_span(span) {}
        iterator begin() { return iterator(m_span.begin()); }
        iterator end() { return iterator(m_span.end()); }
        size_t size() const { return m_span.size(); }
        bool empty() const { return m_span.empty(); }
        node& operator[](size_t i) { EXPECTS(i < m_span.size()); return *m_span[i]; } // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        const node& operator[](size_t i) const { EXPECTS(i < m_span.size()); return *m_span[i]; } // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    };
}

#endif // ENGINE_SCENE_NODE_SPAN_HPP
