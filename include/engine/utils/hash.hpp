#ifndef ENGINE_UTILS_HASH_HPP
#define ENGINE_UTILS_HASH_HPP

#include <ankerl/unordered_dense.h>
#include <unordered_set>
#include <unordered_map>
#include <glm/glm.hpp>

namespace engine {
    template<class K>
    using hashset = ankerl::unordered_dense::set<K, ankerl::unordered_dense::hash<K>>;

    template<class K, class V>
    using hashmap = ankerl::unordered_dense::map<K, V, ankerl::unordered_dense::hash<K>>;
}

// ankerl can handle hashing tuples, so we can define hashing glm::vec in terms of hashing a tuple of its contents
template<typename T, int N, glm::qualifier Q>
struct ankerl::unordered_dense::hash<glm::vec<N, T, Q>> {
    hash() = default;
    std::size_t operator()(const glm::vec<N, T, Q>& v) const noexcept {
        std::array<T,N> arr;
        for(int i = 0; i < N; i++)
            arr[i] = v[i];
        auto tuple = std::tuple_cat(arr);
        return ankerl::unordered_dense::hash<decltype(tuple)>{}(tuple);
    }
};


#endif // ENGINE_UTILS_HASH_HPP
