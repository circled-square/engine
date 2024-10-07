#ifndef ENGINE_RC_HPP
#define ENGINE_RC_HPP

#include "internal/rc_resource.hpp"

namespace engine {
    //mutable template specialization
    template<typename T>
    class rc {
        rc_resource<T>* m_resource;

        //friends which use the private constructor
        friend class resources_manager;
        friend class rc<const T>;


        rc(rc_resource<T>* resource) : m_resource(&*resource) {
            EXPECTS(m_resource);
            m_resource->inc_refcount();
        }
    public:
        rc() : m_resource(nullptr) {}
        rc(const rc& o) : rc() {
            this->operator=(o);
        }

        rc(rc&& o) : m_resource(o.m_resource) {
            o.m_resource = nullptr;
        }
        ~rc() {
            if(m_resource) {
                m_resource->dec_refcount();

                m_resource = nullptr;
            }
        }

        rc& operator=(const rc& o) {
            if(this->m_resource)
                this->m_resource->dec_refcount();

            this->m_resource = o.m_resource;
            if(this->m_resource)
                this->m_resource->inc_refcount();

            return *this;
        }

        operator bool() const { return m_resource; }
        bool operator!() const { return !m_resource; }

        T& operator*() {
            EXPECTS(m_resource);
            return m_resource->resource();
        }
        const T& operator*() const {
            EXPECTS(m_resource);
            return m_resource->resource();
        }
        T* operator->() {
            EXPECTS(m_resource);
            return &m_resource->resource();
        }
        const T* operator->() const {
            EXPECTS(m_resource);
            return &m_resource->resource();
        }

        bool operator==(const rc& o) const { return m_resource == o.m_resource; }
        bool operator!=(const rc& o) const { return m_resource != o.m_resource; }
    };


    //immutable template specialization
    template<typename T>
    class rc<const T> : rc<T> {
        friend class resources_manager;

        //constructor used by resource manager
        rc(rc_resource<T>* resource) : rc<T>(resource) {}
    public:
        rc() : rc<T>() {}
        rc(const rc& o) : rc<T>(o) {}
        rc(rc&& o) : rc<T>(std::move(o)) {}
        rc(rc<T> o) : rc<T>(std::move(o)) {}

        rc& operator=(const rc& o) {
            this->rc<T>::operator=(o);
            return *this;
        }

        operator bool() const { return rc<T>::operator bool(); }
        bool operator!() const { return rc<T>::operator!(); }

        const T& operator*() const { return rc<T>::operator*(); }
        const T* operator->() const { return rc<T>::operator->(); }

        bool operator==(const rc& o) const { return rc<T>::operator==(o); }
        bool operator!=(const rc& o) const { return rc<T>::operator!=(o); }
    };
}
#endif // ENGINE_RC_HPP
