#ifndef ENGINE_RESOURCES_MANAGER_DETAIL_TYPEDEFS_HPP
#define ENGINE_RESOURCES_MANAGER_DETAIL_TYPEDEFS_HPP

#include <memory>
#include "resource_id.hpp"
#include "../resource_concept.hpp"
#include <engine/utils/meta.hpp>
#include <engine/utils/hash.hpp>

namespace engine::detail {
    // An owning pointer with reference counting, can be used to construct a rc
    template<Resource T> using rc_ptr = std::unique_ptr<rc_resource<T>>;

    using resource_name_t = std::variant<std::monostate, std::string, uint8_t>;
    // these aliases are necessary to compose complex types from resource_tuple_t with map_tuple
    // map from id to (name, resource)
    template<Resource T> using id_to_resource_hashmap = hashmap<resource_id<T>, std::tuple<resource_name_t, rc_ptr<T>>>;
    // map from name to id
    template<Resource T> using name_to_id_hashmap = hashmap<resource_name_t, resource_id<T>>;
    // set of ids for GCion
    template<Resource T> using id_hashset = hashset<resource_id<T>>;

    using id_to_resources_hashmaps = map_tuple<id_to_resource_hashmap, resource_tuple_t>;
    using name_to_id_hashmaps = map_tuple<name_to_id_hashmap, resource_tuple_t>;
    using id_hashsets = map_tuple<id_hashset, resource_tuple_t>;

    class resources_manager_hashmaps {
        id_to_resources_hashmaps m_id_to_resource;
        name_to_id_hashmaps m_name_to_id;
        id_hashsets m_marked_for_deletion;
    public:
        template<Resource T> id_to_resource_hashmap<T>& id_to_resource() { return std::get<id_to_resource_hashmap<T>>(m_id_to_resource); }
        template<Resource T> name_to_id_hashmap<T>& name_to_id() { return std::get<name_to_id_hashmap<T>>(m_name_to_id); }
        template<Resource T> id_hashset<T>& marked_for_deletion() { return std::get<id_hashset<T>>(m_marked_for_deletion); }

        id_hashsets& all_marked_for_deletion_sets() { return m_marked_for_deletion; }
    };
}

#endif // ENGINE_RESOURCES_MANAGER_DETAIL_TYPEDEFS_HPP
