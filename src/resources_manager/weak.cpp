#include <engine/resources_manager/weak.hpp>
#include <engine/scene.hpp>

namespace engine {
    template<Resource T>
    weak<T>::weak(detail::rc_resource<T>* resource) : m_resource(resource) {
        EXPECTS(resource);
        m_resource->inc_weak_refcount();
    }

    template<Resource T>
    weak<T>::weak() : m_resource(nullptr) {}

    template<Resource T>
    weak<T>::weak(std::nullptr_t) : weak() {}

    template<Resource T>
    weak<T>::weak(const weak<T>& o) : weak(o.m_resource) {}

    template<Resource T>
    weak<T>::weak(const rc<T>& o) : weak(o.get_resource_ptr()) {}

    template<Resource T>
    weak<T>::weak(weak<T>&& o) : m_resource(o.m_resource) {
        o.m_resource = nullptr;
    }

    template<Resource T>
    weak<T>::~weak() {
        if(m_resource) {
            m_resource->dec_weak_refcount();

            m_resource = nullptr;
        }
    }

    template<Resource T>
    weak<T>& weak<T>::operator=(const weak<T>& o) {
        if(this->m_resource)
            this->m_resource->dec_weak_refcount();

        this->m_resource = o.m_resource;
        if(this->m_resource)
            this->m_resource->inc_weak_refcount();

        return *this;
    }

    template<Resource T>
    rc<T> weak<T>::lock() const {
        if(m_resource && m_resource->resource())
            return rc<T>(m_resource);
        else
            return rc<T>();
    }



    template<Resource T>
    bool weak<T>::operator==(const weak<T>& o) const { return m_resource == o.m_resource; }

    template<Resource T>
    bool weak<T>::operator!=(const weak<T>& o) const { return m_resource != o.m_resource; }

    template<Resource T>
    weak<const T>::weak(detail::rc_resource<T>* resource) : weak<T>(resource) {}

    template<Resource T>
    weak<const T>::weak() : weak<T>() {}

    template<Resource T>
    weak<const T>::weak(std::nullptr_t) : weak() {}

    template<Resource T>
    weak<const T>::weak(const weak<const T>& o) : weak<T>(o) {}

    template<Resource T>
    weak<const T>::weak(weak<const T>&& o) : weak<T>(std::forward<weak&&>(o)) {}

    template<Resource T>
    weak<const T>::weak(weak<T> o) : weak<T>(std::move(o)) {}

    template<Resource T>
    weak<const T>& weak<const T>::operator=(const weak<const T>& o) {
        this->weak<T>::operator=(o);
        return *this;
    }

    template<Resource T>
    rc<const T> weak<const T>::lock() const {
        return this->weak<T>::lock();
    }

    template<Resource T>
    bool weak<const T>::operator==(const weak<const T>& o) const { return this->weak<T>::operator==(o); }

    template<Resource T>
    bool weak<const T>::operator!=(const weak<const T>& o) const { return this->weak<T>::operator!=(o); }


    #define INSTANTIATE_WEAK_TEMPLATE(TYPE) \
        template class weak<TYPE>; \
        template class weak<const TYPE>;

    CALL_MACRO_FOR_EACH(INSTANTIATE_WEAK_TEMPLATE, RESOURCES)
}
