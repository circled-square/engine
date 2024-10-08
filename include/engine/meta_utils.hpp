#ifndef ENGINE_META_UTILS_HPP
#define ENGINE_META_UTILS_HPP

#include <type_traits>
#include <concepts>
#include <tuple>
#include <variant>

// general purpose concepts: Polymorphic, Derived, ContainedInTuple, ContainedInVariant
namespace engine {
    template<typename T>
    concept Polymorphic = std::is_polymorphic_v<T>;
    template<class T, class U>
    concept Derived = std::is_base_of<U, T>::value;


    template<typename T, typename... Ts>
    concept AnyOneOf = (std::same_as<T, Ts> || ...);

    namespace detail {
        template<typename T, template<typename...>typename generic_tuple_t, typename tuple_t>
        struct contained_in_pack__struct {
            static_assert(false, "tuple_t must be an instantiation of generic_tuple_t");
        };
        template<typename T, template<typename...>typename generic_tuple_t, typename...Ts>
        struct contained_in_pack__struct<T, generic_tuple_t, generic_tuple_t<Ts...>> {
            static constexpr bool value = AnyOneOf<T, Ts...>;
        };
    }

    template<typename T, typename tuple_t>
    concept ContainedInTuple = detail::contained_in_pack__struct<T, std::tuple, tuple_t>::value;

    template<typename T, typename variant_t>
    concept ContainedInVariant = detail::contained_in_pack__struct<T, std::variant, variant_t>::value;

    namespace detail {
        //map_tuple maps all element types of a given Tuple type with the Template template
        template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class Tuple>
        struct map_pack__struct {
            static_assert(false, "map_tuple's Tuple argument should be an instantiation of std::tuple");
        };
        template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class... Ts>
        struct map_pack__struct<Template, InTuple, OutTuple, InTuple<Ts...>> {
            using type = OutTuple<Template<Ts>...>;
        };
    }

    template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class Tuple>
    using map_pack = detail::map_pack__struct<Template, InTuple, OutTuple, Tuple>::type;

    template<template<class> class Template, class Tuple> using map_tuple = map_pack<Template, std::tuple, std::tuple, Tuple>;

}
#endif // ENGINE_META_UTILS_HPP
