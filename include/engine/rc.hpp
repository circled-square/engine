#ifndef ENGINE_RC_HPP
#define ENGINE_RC_HPP

#include "internal/rc_resource.hpp"
#include <type_traits>

// rc is a reference counted pointer managed by the engine's resources manager, which points to a Resource

namespace engine {
    // unused base template
    template<typename T>
    class rc {
        static_assert(false, "rc<T>: T must be a Resource or const Resource type");
    };

    // mutable resource template specialization
    template<Resource T>
    class rc<T> {
        detail::rc_resource<T>* m_resource;

        //friends which use the private constructor
        friend class resources_manager;
        friend class rc<const T>;


        rc(detail::rc_resource<T>* resource);
    public:
        rc();
        rc(const rc& o);

        rc(rc&& o);
        ~rc();

        rc& operator=(const rc& o);

        operator bool() const;
        bool operator!() const;

        T& operator*();
        const T& operator*() const;
        T* operator->();
        const T* operator->() const;

        bool operator==(const rc& o) const;
        bool operator!=(const rc& o) const;
    };


    // immutable resource template specialization
    template<Resource T>
    class rc<const T> : rc<T> {
        friend class resources_manager;

        //constructor used by resource manager
        rc(detail::rc_resource<T>* resource);
    public:
        rc();
        rc(const rc& o);
        rc(rc&& o);
        rc(rc<T> o);

        rc& operator=(const rc& o);

        operator bool() const;
        bool operator!() const;

        const T& operator*() const;
        const T* operator->() const;

        bool operator==(const rc& o) const;
        bool operator!=(const rc& o) const;
    };
}
#endif // ENGINE_RC_HPP
