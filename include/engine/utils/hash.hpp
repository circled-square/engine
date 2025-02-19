#ifndef ENGINE_UTILS_MURMURHASH_HPP
#define ENGINE_UTILS_MURMURHASH_HPP

#include <MurmurHash3.h>
#include <string> // also includes std::hash, which does not have its own header for some reason
#include <ankerl/unordered_dense.h>
#include <unordered_set>
#include <unordered_map>
#include <glm/glm.hpp>

namespace engine {
    template<typename T>
    concept StdHashable = requires(T a) {
        { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
    };
    template<typename T, int N>
    concept GlmVector = false;

    template<class T>
    struct murmurhash3;

    template<StdHashable T>
    struct murmurhash3<T> {
        murmurhash3() = default;
        size_t operator()(const T& v) const noexcept {
            std::size_t h = std::hash<T>()(v);

            std::uint64_t result[2];
            MurmurHash3_x64_128(&h, sizeof(h), 0, &result);

            return result[0] ^ result[1];
        }
    };

    template<typename T, int N, glm::qualifier Q>
    struct murmurhash3<glm::vec<N, T, Q>> {
        murmurhash3() = default;
        std::size_t operator()(const glm::vec<N, T, Q>& v) const noexcept {
            std::uint64_t result[2];
            MurmurHash3_x64_128(&v, sizeof(v[0]) * N, 0, &result);
            return result[0] ^ result[1];
        }
    };


    template<class K>
    using hashset = ankerl::unordered_dense::set<K>; //std::unordered_set<K, murmurhash3<K>>;

    template<class K, class V>
    using hashmap = ankerl::unordered_dense::map<K, V>; //std::unordered_map<K, V, murmurhash3<K>>;
}

#endif // ENGINE_UTILS_MURMURHASH_HPP
