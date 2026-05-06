#ifndef ENGINE_UTILS_OPTIONAL_REF_HPP
#define ENGINE_UTILS_OPTIONAL_REF_HPP

namespace engine {
    template<typename T>
    class optional_ref {
        T* m_p;
    public:
        optional_ref() : m_p(nullptr) {}
        optional_ref(T& v) : m_p(&v) {}
        operator bool() { return m_p != nullptr; }
        bool operator!() { return m_p == nullptr; }
        T& operator*() { EXPECTS(m_p != nullptr); return *m_p; }
    };
}

#endif // ENGINE_UTILS_OPTIONAL_REF_HPP
