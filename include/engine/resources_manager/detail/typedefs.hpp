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

    // these aliases are necessary to compose complex types from resource_tuple_t with map_tuple
    // map from id to (name, resource)
    template<Resource T> using id_to_resource_hashmap = hashmap<resource_id<T>, std::pair<std::string, rc_ptr<T>>>;
    // map from name to id
    template<Resource T> using name_to_id_hashmap = hashmap<std::string, resource_id<T>>;
    // set of ids for GCion
    template<Resource T> using id_hashset = hashset<resource_id<T>>;

    using id_to_resources_hashmaps = map_tuple<id_to_resource_hashmap, resource_tuple_t>;
    using name_to_id_hashmaps = map_tuple<name_to_id_hashmap, resource_tuple_t>;
    using id_hashsets = map_tuple<id_hashset, resource_tuple_t>;

}

#endif // ENGINE_RESOURCES_MANAGER_DETAIL_TYPEDEFS_HPP
