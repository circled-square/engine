#ifndef ENGINE_META_UTILS_HPP
#define ENGINE_META_UTILS_HPP

#include <concepts>
#include <tuple>
#include <variant>

// general purpose concepts and utilities: AnyOneOf, ContainedInTuple, ContainedInVariant, map_pack, map_tuple, match_variant
namespace engine {
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

    //takes a Tuple = InTuple<Ts...> type and returns OutTuple<Template<Ts>...> type
    template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class Tuple>
    using map_pack = detail::map_pack__struct<Template, InTuple, OutTuple, Tuple>::type;

    //takes a Tuple=std::tuple<Ts...> type and returns std::tuple<Template<Ts>...>
    template<template<class> class Template, class Tuple> using map_tuple = map_pack<Template, std::tuple, std::tuple, Tuple>;

    namespace detail {
        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };
        // explicit deduction guide (not needed as of C++20)
        template<class... Ts>
        overloaded(Ts...) -> overloaded<Ts...>;
    }

    //like std::visit but with multiple overloads; will cause a compile error if not all of the variant's cases are covered
    template<typename...Ts>
    void match_variant(const std::variant<Ts...>& v, const auto&... handlers) {
        std::visit(detail::overloaded(handlers...), v);
    }


    // CALL_MACRO_FOR_EACH: a for each implemented in the preprocessor
    // expand to the Nth arg; N = 10
    #define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N

    // for each macros for different number of arguments
    #define __FE_0(MACRO, ...)
    #define __FE_1(MACRO, x) MACRO(x)
    #define __FE_2(MACRO, x, ...) MACRO(x) __FE_1(MACRO, __VA_ARGS__)
    #define __FE_3(MACRO, x, ...) MACRO(x) __FE_2(MACRO, __VA_ARGS__)
    #define __FE_4(MACRO, x, ...) MACRO(x) __FE_3(MACRO, __VA_ARGS__)
    #define __FE_5(MACRO, x, ...) MACRO(x) __FE_4(MACRO, __VA_ARGS__)
    #define __FE_6(MACRO, x, ...) MACRO(x) __FE_5(MACRO, __VA_ARGS__)
    #define __FE_7(MACRO, x, ...) MACRO(x) __FE_6(MACRO, __VA_ARGS__)
    #define __FE_8(MACRO, x, ...) MACRO(x) __FE_7(MACRO, __VA_ARGS__)
    #define __FE_9(MACRO, x, ...) MACRO(x) __FE_8(MACRO, __VA_ARGS__)

    #define CALL_MACRO_FOR_EACH(MACRO,...) _GET_NTH_ARG(__VA_ARGS__, __FE_9, __FE_8, __FE_7, __FE_6, __FE_5, __FE_4, __FE_3, __FE_2, __FE_1, __FE_0)(MACRO, __VA_ARGS__)
}
#endif // ENGINE_META_UTILS_HPP
