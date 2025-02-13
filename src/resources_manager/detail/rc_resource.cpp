#include <engine/resources_manager/detail/rc_resource.hpp>
#include <engine/scene.hpp> // to instantiate rc_resource<scene>
#include <engine/scene/node.hpp>
#include <engine/resources_manager/detail/resource_id.hpp>

namespace engine::detail {
    template<Resource T>
    int64_t rc_resource<T>::refcount() const { return m_refcount; }

    template<Resource T>
    void rc_resource<T>::inc_refcount() {
        m_refcount++;
    }

    template<Resource T>
    void rc_resource<T>::dec_refcount() {
        EXPECTS(m_refcount > 0);
        m_refcount--;
        if(m_refcount <= 0) {
            engine::flag_for_deletion(get_rm(), resource_id(this));
        }
    }

    template<Resource T>
    int64_t rc_resource<T>::weak_refcount() const { return m_weak_refcount; }

    template<Resource T>
    void rc_resource<T>::inc_weak_refcount() {
        m_weak_refcount++;
    }

    template<Resource T>
    void rc_resource<T>::dec_weak_refcount() {
        EXPECTS(m_weak_refcount > 0);
        m_weak_refcount--;
        if(m_weak_refcount <= 0) {
            engine::flag_for_deletion(get_rm(), resource_id(this));
        }
    }

    template<Resource T>
    std::optional<T>& rc_resource<T>::resource() { return m_resource; }

    #define INSTANTIATE_RC_RESOURCE_TEMPLATE(TYPE) \
        template class rc_resource<TYPE>;

    CALL_MACRO_FOR_EACH(INSTANTIATE_RC_RESOURCE_TEMPLATE, RESOURCES)
}
