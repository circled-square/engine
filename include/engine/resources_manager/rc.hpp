#ifndef ENGINE_RESOURCES_MANAGER_RC_HPP
#define ENGINE_RESOURCES_MANAGER_RC_HPP

#include "detail/rc_resource.hpp"
#include <engine/utils/api_macro.hpp>
#include <slogga/asserts.hpp>

// rc is a reference counted pointer managed by the engine's resources manager, which points to a Resource

namespace engine {
    template<typename T>
    class rc;

    template<class T> class weak;

    // mutable resource template specialization
    template<Resource T>
    class ENGINE_API rc<T> {
        detail::rc_resource<T>* m_resource;

        // friends which use the private constructor
        friend class resources_manager;
        friend class rc<const T>;

        // needs to get the resource pointer to be able to construct from an rc,
        // and uses the private constructor to lock() itself into an rc
        friend class weak<T>;

        rc(detail::rc_resource<T>* resource);
    protected:
        detail::rc_resource<T>* get_resource_ptr() const;
    public:
        using element_type = T;
        using mut_element_type = T;

        rc() = delete;
        rc(std::nullptr_t) = delete;

        ENGINE_API rc(const rc& o);

        ENGINE_API rc(rc&& o);
        ENGINE_API ~rc();

        ENGINE_API rc& operator=(const rc& o);

        //note: dereferencing a const rc yields a mutable object!
        ENGINE_API T& operator*() const;
        ENGINE_API T* operator->() const;

        //always returns true, since it is never null except after move
        operator bool() const { EXPECTS(m_resource); return true; }
        //always returns false, since it is never null except after move
        bool operator!() const { EXPECTS(m_resource); return false;}
    private: // are these even needed? & same for the ones in rc<const>, weak, weak<const>
        bool operator==(const rc& o) const;
        bool operator!=(const rc& o) const;
    };


    // immutable resource template specialization
    template<Resource T>
    class rc<const T> : rc<T> {
        friend class resources_manager;

        friend class weak<const T>;

        //constructor used by resource manager
        rc(detail::rc_resource<T>* resource);
    public:
        using element_type = const T;
        using mut_element_type = T;

        rc() = delete;
        rc(std::nullptr_t) = delete;

        ENGINE_API rc(const rc& o);
        ENGINE_API rc(rc&& o);
        ENGINE_API rc(const rc<T>& o);
        ENGINE_API rc(rc<T>&& o);

        ENGINE_API rc& operator=(const rc& o);

        ENGINE_API const T& operator*() const;
        ENGINE_API const T* operator->() const;

        //always returns true, since it is never null except after move
        operator bool() const { return rc<T>::operator bool(); }
        //always returns false, since it is never null except after move
        bool operator!() const { return rc<T>::operator!(); }
    private:
        bool operator==(const rc& o) const;
        bool operator!=(const rc& o) const;
    };


    template<typename T>
    class nullable_rc;

    template<Resource T>
    class nullable_rc<T> {
        std::optional<rc<T>> m_ptr;

        friend class nullable_rc<const T>;
    public:
        using mut_element_type = std::remove_const_t<T>;
        using element_type = T;
        using const_element_type = const T;

        nullable_rc() = default;
        nullable_rc(std::nullopt_t) : nullable_rc() {}
        nullable_rc(rc<element_type>&& o) : m_ptr(std::move(o)) {}
        nullable_rc(const rc<element_type>& o) : m_ptr(o) {}

        nullable_rc(const nullable_rc& o) = default;
        nullable_rc(nullable_rc&& o) = default;

        nullable_rc& operator=(const nullable_rc& o) = default;

        element_type& operator*() const {
            //TODO: add exception throw on null
            EXPECTS(this->m_ptr.has_value());
            return **m_ptr;
        }
        element_type* operator->() const {
            //TODO: add exception throw on null
            EXPECTS(this->m_ptr.has_value());
            return m_ptr->operator->();
        }
        rc<element_type> as_nonnull() const {
            //TODO: add exception throw on null
            EXPECTS(this->m_ptr.has_value());
            return *m_ptr;
        }

        operator bool() const { return m_ptr.has_value(); }
        bool operator!() const { return !m_ptr.has_value(); }
    };

    //only difference from the mut counterpart is that it can be constructed from it but not vice versa
    template<Resource T>
    class nullable_rc<const T> {
        std::optional<rc<const T>> m_ptr;
    public:
        using mut_element_type = std::remove_const_t<T>;
        using element_type = const T;
        using const_element_type = const T;

        nullable_rc() = default;
        nullable_rc(std::nullopt_t) : nullable_rc() {}
        nullable_rc(rc<element_type>&& o) : m_ptr(std::move(o)) {}
        nullable_rc(const rc<element_type>& o) : m_ptr(o) {}

        nullable_rc(const nullable_rc& o) = default;
        nullable_rc(nullable_rc&& o) = default;

        nullable_rc& operator=(const nullable_rc& o) = default;

        nullable_rc(const nullable_rc<mut_element_type>& o) : m_ptr(o.m_ptr) {}
        nullable_rc(nullable_rc<mut_element_type>&& o) : m_ptr(o.m_ptr) {}

        element_type& operator*() const {
            //TODO: add exception throw on null
            EXPECTS(this->m_ptr.has_value());
            return **m_ptr;
        }
        element_type* operator->() const {
            //TODO: add exception throw on null
            EXPECTS(this->m_ptr.has_value());
            return m_ptr->operator->();
        }
        rc<element_type> as_nonnull() const {
            //TODO: add exception throw on null
            EXPECTS(this->m_ptr.has_value());
            return *m_ptr;
        }

        operator bool() const { return m_ptr.has_value(); }
        bool operator!() const { return !m_ptr.has_value(); }
    };
}
#endif // ENGINE_RESOURCES_MANAGER_RC_HPP
