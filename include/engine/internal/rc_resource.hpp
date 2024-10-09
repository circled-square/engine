#ifndef ENGINE_INTERNAL_RC_RESOURCE_HPP
#define ENGINE_INTERNAL_RC_RESOURCE_HPP

#include "../concepts.hpp"
#include <cstdint>
#include <slogga/asserts.hpp>

// rc_resource<T>: internal type for rc and resources_manager.
// where rc is the pointer to the resource, rc_resource is its allocation, containing also the reference count

namespace engine {
    class resources_manager;
    resources_manager& get_rm();
    template<Resource T>
    class rc_resource;

    template<typename T>
    void flag_for_deletion(resources_manager& rm, rc_resource<T>* resource);

    template<Resource T>
    class rc_resource {
        // rc_resource itself does not enforce immutability of the contained resource; it is up to its owner to do it
        T m_resource;
        std::int64_t m_refcount;

    public:
        template<typename... Args>
        rc_resource(Args&&... args) : m_resource(std::forward<Args>(args)...), m_refcount(0) {}

        rc_resource(rc_resource&&) = delete;
        rc_resource(const rc_resource&) = delete;
        rc_resource& operator=(rc_resource&&) = delete;
        rc_resource& operator=(const rc_resource&) = delete;

        std::int64_t refcount() const { return m_refcount; }
        void inc_refcount() {
            m_refcount++;
        }
        void dec_refcount() {
            EXPECTS(m_refcount > 0);
            m_refcount--;
            if(m_refcount <= 0) {
                flag_for_deletion(get_rm(), this);
            }
        }

        T& resource() { return m_resource; }
    };
}

#endif // ENGINE_INTERNAL_RC_RESOURCE_HPP
