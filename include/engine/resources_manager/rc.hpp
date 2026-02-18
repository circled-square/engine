#ifndef ENGINE_RESOURCES_MANAGER_RC_HPP
#define ENGINE_RESOURCES_MANAGER_RC_HPP

#include "detail/rc_resource.hpp"
#include <engine/utils/api_macro.hpp>

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

        ENGINE_API rc();
        ENGINE_API rc(std::nullptr_t);

        ENGINE_API rc(const rc& o);

        ENGINE_API rc(rc&& o);
        ENGINE_API ~rc();

        ENGINE_API rc& operator=(const rc& o);

        ENGINE_API operator bool() const;
        ENGINE_API bool operator!() const;

        //note: dereferencing a const rc yields a mutable object!
        ENGINE_API T& operator*() const;
        ENGINE_API T* operator->() const;

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

        ENGINE_API rc();
        ENGINE_API rc(std::nullptr_t);
        ENGINE_API rc(const rc& o);
        ENGINE_API rc(rc&& o);
        ENGINE_API rc(const rc<T>& o);
        ENGINE_API rc(rc<T>&& o);

        ENGINE_API rc& operator=(const rc& o);

        ENGINE_API operator bool() const;
        ENGINE_API bool operator!() const;

        ENGINE_API const T& operator*() const;
        ENGINE_API const T* operator->() const;

    private:
        bool operator==(const rc& o) const;
        bool operator!=(const rc& o) const;
    };



    #define INSTANTIATE_RC_TEMPLATE(TYPE) \
        template class rc<TYPE>; \
        template class rc<const TYPE>;

    CALL_MACRO_FOR_EACH(INSTANTIATE_RC_TEMPLATE, RESOURCES)


}
#endif // ENGINE_RESOURCES_MANAGER_RC_HPP
