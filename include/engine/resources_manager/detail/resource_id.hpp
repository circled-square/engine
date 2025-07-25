#ifndef ENGINE_RESOURCES_MANAGER_DETAIL_RESOURCE_ID_HPP
#define ENGINE_RESOURCES_MANAGER_DETAIL_RESOURCE_ID_HPP

#include "rc_resource.hpp"
#include "../resource_concept.hpp"

namespace engine::detail {
    // The id of a resource, which can be used to retrieve from rm's hashmaps the resource itself, its name and whether it is flagged for deletion.
    // Note: To be unique to the resource, tied to its type, for hashing and for simplicity a ptr could have been used;
    // however a resource_id is not meant to be derefed or used in ptr arithmetic, so this was deemed inappropriate
    template<Resource T>
    class resource_id {
        std::uintptr_t m_id; // if m_id is 0 the id is null
        friend class std::hash<resource_id>;
    public:
        resource_id() : m_id(0) {}
        resource_id(const rc_resource<T>* resource) : m_id((std::uintptr_t)resource) {}
        resource_id(const rc_resource<T>& resource) : resource_id(&resource) {}
        resource_id(const resource_id& o) : m_id(o.m_id) {}
        bool operator==(const resource_id& o) const { return m_id == o.m_id; }
    };
}

template<engine::Resource T>
struct std::hash<engine::detail::resource_id<T>> {
    std::size_t operator()(const engine::detail::resource_id<T>& rid) const noexcept {
        return std::hash<std::uintptr_t>{}(rid.m_id);
    };
};

#endif // ENGINE_RESOURCES_MANAGER_DETAIL_RESOURCE_ID_HPP
