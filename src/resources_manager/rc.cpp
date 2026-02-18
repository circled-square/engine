#include <engine/resources_manager/rc.hpp>
#include <engine/scene.hpp>
#include <dylib.hpp>

namespace engine {
    //private constructor used by resources_manager, rc<const T> & weak<T>
    template<Resource T> ENGINE_API
    rc<T>::rc(detail::rc_resource<T>* resource) : m_resource(resource) {
        EXPECTS(m_resource);
        m_resource->inc_refcount();
    }


    //private getter used by weak<T>
    template<Resource T> ENGINE_API
    detail::rc_resource<T>* rc<T>::get_resource_ptr() const { return m_resource; }

    template<Resource T> ENGINE_API
    rc<T>::rc() : m_resource(nullptr) {}

    template<Resource T> ENGINE_API
    rc<T>::rc(std::nullptr_t) : rc() {}

    template<Resource T> ENGINE_API
    rc<T>::rc(const rc<T>& o) : rc() {
        this->operator=(o);
    }

    template<Resource T> ENGINE_API
    rc<T>::rc(rc<T>&& o) : m_resource(o.m_resource) {
        o.m_resource = nullptr;
    }

    template<Resource T> ENGINE_API
    rc<T>::~rc() {
        if(m_resource) {
            m_resource->dec_refcount();

            m_resource = nullptr;
        }
    }

    template<Resource T> ENGINE_API
    rc<T>& rc<T>::operator=(const rc<T>& o) {
        if(this->m_resource)
            this->m_resource->dec_refcount();

        this->m_resource = o.m_resource;
        if(this->m_resource)
            this->m_resource->inc_refcount();

        return *this;
    }

    template<Resource T> ENGINE_API
    rc<T>::operator bool() const { return m_resource; }

    template<Resource T> ENGINE_API
    bool rc<T>::operator!() const { return !m_resource; }

    template<Resource T> ENGINE_API
    T& rc<T>::operator*() const {
        EXPECTS(m_resource);
        EXPECTS(m_resource->resource().has_value());
        return *m_resource->resource();
    }

    template<Resource T> ENGINE_API
    T* rc<T>::operator->() const {
        EXPECTS(m_resource);
        EXPECTS(m_resource->resource().has_value());
        return &*m_resource->resource();
    }

    template<Resource T> ENGINE_API
    bool rc<T>::operator==(const rc<T> &o) const { return m_resource == o.m_resource; }

    template<Resource T> ENGINE_API
    bool rc<T>::operator!=(const rc<T> &o) const { return m_resource != o.m_resource; }





    //private constructor used by resources_manager
    template<Resource T> ENGINE_API
    rc<const T>::rc(detail::rc_resource<T> *resource) : rc<T>(resource) {}

    template<Resource T> ENGINE_API
    rc<const T>::rc() : rc<T>() {}

    template<Resource T> ENGINE_API
    rc<const T>::rc(std::nullptr_t) : rc() {}

    template<Resource T> ENGINE_API
    rc<const T>::rc(const rc<T>& o) : rc<T>(o) {}

    template<Resource T> ENGINE_API
    rc<const T>::rc(rc<T>&& o) : rc<T>(std::forward<rc<T>&&>(o)) {}

    template<Resource T> ENGINE_API
    rc<const T>::rc(rc<const T>&& o) : rc<T>(std::move(o)) {}

    template<Resource T> ENGINE_API
    rc<const T>::rc(const rc<const T>& o) : rc<T>(o) {}


    template<Resource T> ENGINE_API
    rc<const T> &rc<const T>::operator=(const rc<const T> &o) {
        this->rc<T>::operator=(o);
        return *this;
    }


    template<Resource T> ENGINE_API
    const T* rc<const T>::operator->() const { return rc<T>::operator->(); }

    template<Resource T> ENGINE_API
    const T& rc<const T>::operator*() const { return rc<T>::operator*(); }


    template<Resource T> ENGINE_API
    bool rc<const T>::operator!() const { return rc<T>::operator!(); }

    template<Resource T> ENGINE_API
    rc<const T>::operator bool() const { return rc<T>::operator bool(); }


    template<Resource T> ENGINE_API
    bool rc<const T>::operator!=(const rc<const T> &o) const { return rc<T>::operator!=(o); }

    template<Resource T> ENGINE_API
    bool rc<const T>::operator==(const rc<const T> &o) const { return rc<T>::operator==(o); }


    // #define INSTANTIATE_RC_TEMPLATE(TYPE) \
        // template class rc<TYPE>; \
        // template class rc<const TYPE>;

    // CALL_MACRO_FOR_EACH(INSTANTIATE_RC_TEMPLATE, RESOURCES)
}
