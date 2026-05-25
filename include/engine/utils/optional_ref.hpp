#ifndef ENGINE_UTILS_OPTIONAL_REF_HPP
#define ENGINE_UTILS_OPTIONAL_REF_HPP

#include <optional>
#include <slogga/asserts.hpp>

namespace engine {
    template<typename T>
    class optional_ref {
        T* m_p;
    public:
        optional_ref() : m_p(nullptr) {}
        optional_ref(T& v) : m_p(&v) {}
        bool has_value() { return m_p != nullptr; }
        operator bool() { return has_value(); }
        bool operator!() { return !has_value(); }

        T& value() { EXPECTS(has_value()); return *m_p; }
        T& operator*() { return value(); }
        T* operator->() { EXPECTS(has_value()); return m_p; }

        operator std::optional<T>() const {
            if(has_value()) {
                return value();
            } else {
                return std::nullopt;
            }
        }
    };
}

#endif // ENGINE_UTILS_OPTIONAL_REF_HPP
