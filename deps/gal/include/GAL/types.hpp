#ifndef GAL_INTERNAL_TYPES_HPP
#define GAL_INTERNAL_TYPES_HPP

#include <cassert>
#include <concepts>
#include <cstdint>
#include <glm/glm.hpp>

namespace gal {
    using sbyte = signed char;
    using ubyte = unsigned char;
    using sshort = std::int16_t;
    using ushort = std::uint16_t;
    using sint = std::int32_t;
    using uint = std::uint32_t;
    using int64 = std::int64_t;
    using uint64 = std::uint64_t;
    // it is unnecessary to define float and double, as they are trivially defined the same way by c++ and OpenGL
    // for simplicity bool is used directly, even though the c++ standard does not guarantee that it is 1 byte in size (which OpenGL requires)

    //make a struct for each of the types that OpenGL defines as aliases of other types,
    //so they can be treated as distinct types and passed to templates
    struct [[gnu::packed]] alignas(std::int32_t) fixed { std::int32_t v; };
    struct [[gnu::packed]] alignas(std::uint16_t) half { std::uint16_t v; };

    template <typename T, typename...O>
    concept AnyOf = (std::same_as<T, O> || ...);

    namespace detail {
        template<AnyOf<sbyte, ubyte, sshort, ushort, sint, uint, float, double, half, fixed, bool> T>
        struct gl_type_id__struct;

        template<> struct gl_type_id__struct<sbyte> { static constexpr uint v = 0x1400; };
        template<> struct gl_type_id__struct<ubyte> { static constexpr uint v = 0x1401; };
        template<> struct gl_type_id__struct<sshort> { static constexpr uint v = 0x1402; };
        template<> struct gl_type_id__struct<ushort> { static constexpr uint v = 0x1403; };
        template<> struct gl_type_id__struct<sint> { static constexpr uint v = 0x1404; };
        template<> struct gl_type_id__struct<uint> { static constexpr uint v = 0x1405; };
        template<> struct gl_type_id__struct<float> { static constexpr uint v = 0x1406; };
        template<> struct gl_type_id__struct<double> { static constexpr uint v = 0x140A; };
        template<> struct gl_type_id__struct<half> { static constexpr uint v = 0x140B; };
        template<> struct gl_type_id__struct<fixed> { static constexpr uint v = 0x140C; };
        template<> struct gl_type_id__struct<bool> { static constexpr uint v = 0x8B56; };
    }
    template<AnyOf<sbyte, ubyte, sshort, ushort, sint, uint, float, double, half, fixed, bool> T>
    constexpr uint gl_type_id = detail::gl_type_id__struct<T>::v;


    constexpr std::size_t typeid_to_size(unsigned id) {
        std::size_t size =
            id == gl_type_id<bool>   ? sizeof(bool) :
            id == gl_type_id<sbyte>  ? sizeof(sbyte) :
            id == gl_type_id<ubyte>  ? sizeof(ubyte) :
            id == gl_type_id<sshort> ? sizeof(sshort) :
            id == gl_type_id<ushort> ? sizeof(ushort) :
            id == gl_type_id<sint>   ? sizeof(sint) :
            id == gl_type_id<uint>   ? sizeof(uint) :
            id == gl_type_id<float>  ? sizeof(float) :
            id == gl_type_id<double> ? sizeof(double) :
            id == gl_type_id<half>   ? sizeof(half) :
            id == gl_type_id<fixed>  ? sizeof(fixed) :
            0;
        assert(size != 0);

        return size;
    }


    template<typename T> concept GlmVector = requires {
        { T::length() } -> std::convertible_to<std::size_t>;
        { T::x } -> std::convertible_to<typename T::value_type>;
    };
    template<typename T> concept GlmMatrix = requires {
        typename T::transpose_type;
    };
    //scalar_to_vector<T> wraps any scalar type into its vec<1, T> form, but doesn't wrap T if it is a glm vector
    namespace detail {
        template<typename T>
        struct scalar_to_vector_helper {
            using type = glm::vec<1, T, glm::defaultp>;
        };
        template<GlmVector vec_t>
        struct scalar_to_vector_helper<vec_t>{
            using type = vec_t;
        };
    }
    
    template<typename T>
    using scalar_to_vector = detail::scalar_to_vector_helper<T>::type;

    //vector_to_scalar<T> transforms any vec<n, T> into T, and returns T if it is a scalar
    template<typename T>
    using vector_to_scalar = scalar_to_vector<T>::value_type;
}

#endif // GAL_INTERNAL_TYPES_HPP
