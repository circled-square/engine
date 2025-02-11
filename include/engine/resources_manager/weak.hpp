#ifndef ENGINE_RESOURCES_MANAGER_WEAK_HPP
#define ENGINE_RESOURCES_MANAGER_WEAK_HPP

#include "detail/rc_resource.hpp"
#include "rc.hpp"
#include <type_traits>

// weak is a weak reference counted pointer managed by the engine's resources manager;
// it can be locked to obtain an rc, which will point to a Resource.

namespace engine {
    // unused base template
    template<typename T>
    class weak {
        static_assert(false, "weak<T>: T must be a Resource or const Resource type");
    };

    // mutable resource template specialization
    template<Resource T>
    class weak<T> {
        detail::rc_resource<T>* m_resource;

        //friend which uses the private constructor
        friend class weak<const T>;

        //resources_manager reads the m_resource ptr
        friend class resources_manager;

        weak(detail::rc_resource<T>* resource);
    public:
        using element_type = T;
        using mut_element_type = T;

        weak();
        weak(std::nullptr_t);
        weak(const weak& o);
        weak(const rc<T>& o);
        weak(weak&& o);
        ~weak();

        weak& operator=(const weak& o);

        // if the weak<T> points to an allocation with a resource that is alive, locks it
        // if it points to null or to an allocation with a destroyed resource, returns null
        rc<T> lock() const;

        bool operator==(const weak& o) const;
        bool operator!=(const weak& o) const;
    };

    // immutable resource template specialization
    template<Resource T>
    class weak<const T> : weak<T> {
        friend class resources_manager;

        //constructor used by resource manager
        weak(detail::rc_resource<T>* resource);
    public:
        using element_type = const T;
        using mut_element_type = T;

        weak();
        weak(std::nullptr_t);
        weak(const weak& o);
        weak(weak&& o);
        weak(weak<T> o);

        weak& operator=(const weak& o);

        rc<const T> lock() const;


        bool operator==(const weak& o) const;
        bool operator!=(const weak& o) const;
    };
}
#endif // ENGINE_RESOURCES_MANAGER_WEAK_HPP
