#ifndef ENGINE_RESOURCES_MANAGER_WEAK_HPP
#define ENGINE_RESOURCES_MANAGER_WEAK_HPP

#include "detail/rc_resource.hpp"
#include "rc.hpp"
#include <engine/utils/api_macro.hpp>

// weak is a weak reference counted pointer managed by the engine's resources manager;
// it can be locked to obtain an rc, which will point to a Resource.

namespace engine {
    template<typename T>
    class weak;

    // mutable resource template specialization
    template<Resource T>
    class weak<T> {
        //m_resource is mutable because this allows us to have lock() be const, while still setting m_resource to null if it is empty.
        mutable detail::rc_resource<T>* m_resource;

        //friend which uses the private constructor
        friend class weak<const T>;

        //resources_manager reads the m_resource ptr
        friend class resources_manager;

        weak(detail::rc_resource<T>* resource);
    public:
        using element_type = T;
        using mut_element_type = T;

        weak();

        ENGINE_API weak(std::nullptr_t);
        ENGINE_API weak(const weak& o);
        ENGINE_API weak(weak&& o);
        ENGINE_API weak(const rc<T>& o);
        ENGINE_API ~weak();

        ENGINE_API weak& operator=(const weak& o);

        // if the weak<T> points to an allocation with a resource that is alive, locks it
        // if it points to null or to an allocation with a destroyed resource, returns null
        ENGINE_API rc<T> lock() const;
    private:

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

        ENGINE_API weak();
        ENGINE_API weak(std::nullptr_t);
        ENGINE_API weak(const weak& o);
        ENGINE_API weak(weak&& o);
        ENGINE_API weak(weak<T> o);
        ENGINE_API weak(const rc<const T>& o);
        ENGINE_API weak(const rc<T>& o);

        ENGINE_API weak& operator=(const weak& o);

        ENGINE_API rc<const T> lock() const;

    private:
        bool operator==(const weak& o) const;
        bool operator!=(const weak& o) const;
    };
}
#endif // ENGINE_RESOURCES_MANAGER_WEAK_HPP
