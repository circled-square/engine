#ifndef ENGINE_RESOURCES_MANAGER_DETAIL_RC_RESOURCE_HPP
#define ENGINE_RESOURCES_MANAGER_DETAIL_RC_RESOURCE_HPP

#include "../resource_concept.hpp"
#include <cstdint>
#include <optional>
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
            std::optional<T> m_resource;
            std::int64_t m_refcount; // number of rc<T> pointing to this allocation
            std::int64_t m_weak_refcount; // number of weak<T> pointing to this allocation

        public:
            // construct with an empty resource
            rc_resource(std::nullopt_t) : m_resource(), m_refcount(0), m_weak_refcount(0) {}

            // construct and emplace the resource with the given args
            template<typename...Args>
            rc_resource(Args&&... args) : rc_resource(std::nullopt) {
                emplace(std::forward<Args>(args)...);
            }

            template<typename... Args>
            void emplace(Args&&... args) {
                m_resource.emplace(std::forward<Args>(args)...);
            }

            rc_resource() = delete;
            rc_resource(rc_resource&&) = delete;
            rc_resource(const rc_resource&) = delete;
            rc_resource& operator=(rc_resource&&) = delete;
            rc_resource& operator=(const rc_resource&) = delete;

            std::int64_t refcount() const;
            void inc_refcount();
            void dec_refcount();

            std::int64_t weak_refcount() const;
            void inc_weak_refcount();
            void dec_weak_refcount();

            std::optional<T>& resource();
        };
    }
}

#endif // ENGINE_RESOURCES_MANAGER_DETAIL_RC_RESOURCE_HPP
