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

    namespace detail {
        template<Resource T> class rc_resource;
    }

    template<Resource T>
    void flag_for_deletion(resources_manager& rm, detail::rc_resource<T>* resource);

    namespace detail {
        template<Resource T>
        class rc_resource {
            // rc_resource does not enforce immutability of the contained resource; it is up to its owner to do it
            T m_resource;
            std::int64_t m_refcount;

        public:
            template<typename...Args>
            rc_resource(Args&&... args) : m_resource(std::forward<Args>(args)...), m_refcount(0) {}

            rc_resource(rc_resource&&) = delete;
            rc_resource(const rc_resource&) = delete;
            rc_resource& operator=(rc_resource&&) = delete;
            rc_resource& operator=(const rc_resource&) = delete;

            std::int64_t refcount() const;
            void inc_refcount();
            void dec_refcount();

            T& resource();
        };
    }
}

#endif // ENGINE_INTERNAL_RC_RESOURCE_HPP
