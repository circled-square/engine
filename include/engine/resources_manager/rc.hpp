#ifndef ENGINE_RESOURCES_MANAGER_RC_HPP
#define ENGINE_RESOURCES_MANAGER_RC_HPP

#include "detail/rc_resource.hpp"
#include <type_traits>

// rc is a reference counted pointer managed by the engine's resources manager, which points to a Resource

namespace engine {
    template<typename T>
    class rc;

    template<class T> class weak;

    // mutable resource template specialization
    template<Resource T>
    class rc<T> {
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

        rc();
        rc(std::nullptr_t);

        rc(const rc& o);

        rc(rc&& o);
        ~rc();

        rc& operator=(const rc& o);

        operator bool() const;
        bool operator!() const;

        //note: dereferencing a const rc yields a mutable object!
        T& operator*() const;
        T* operator->() const;

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

        rc();
        rc(std::nullptr_t);
        rc(const rc& o);
        rc(rc&& o);
        rc(const rc<T>& o);
        rc(rc<T>&& o);

        rc& operator=(const rc& o);

        operator bool() const;
        bool operator!() const;

        const T& operator*() const;
        const T* operator->() const;

        bool operator==(const rc& o) const;
        bool operator!=(const rc& o) const;
    };
}
#endif // ENGINE_RESOURCES_MANAGER_RC_HPP
