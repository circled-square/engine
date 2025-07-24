#ifndef ENGINE_UTILS_DETAIL_META_HPP
#define ENGINE_UTILS_DETAIL_META_HPP

#include <concepts>
#include <tuple>

namespace engine::detail {
    // T is AnyOneOf<Ts...> if T is contained in Ts 
    template<typename T, typename... Ts>
    concept AnyOneOf = (std::same_as<T, Ts> || ...);
    
    // contained_in_pack<T, generic_tuple_t, tuple_t> contains a bool value that is true if tuple_t === generic_tuple_t<..., T, ...>
    template<typename T, template<typename...>typename generic_tuple_t, typename tuple_t>
    struct contained_in_pack__struct {
        static_assert(false, "tuple_t must be an instantiation of generic_tuple_t");
    };
    template<typename T, template<typename...>typename generic_tuple_t, typename...Ts>
    struct contained_in_pack__struct<T, generic_tuple_t, generic_tuple_t<Ts...>> {
        static constexpr bool value = AnyOneOf<T, Ts...>;
    };
    
    //map_tuple maps all element types of a given Tuple type with the Template template
    template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class Tuple>
    struct map_pack__struct {
        static_assert(false, "map_tuple's Tuple argument should be an instantiation of std::tuple");
    };
    template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class... Ts>
    struct map_pack__struct<Template, InTuple, OutTuple, InTuple<Ts...>> {
        using type = OutTuple<Template<Ts>...>;
    };


    template<template<typename...> class TupleTemplate, class Tuple1, class Tuple2>
    struct merge_2_packs__struct {
        static_assert(false, "Tuple1 and Tuple2 must be instantiations of TupleTemplate");
    };
    template<template<typename...> class TupleTemplate, class...Args1, class...Args2>
    struct merge_2_packs__struct<TupleTemplate, TupleTemplate<Args1...>, TupleTemplate<Args2...>> {
        using type = TupleTemplate<Args1..., Args2...>;
    };

    //test
    static_assert(std::same_as<
        std::tuple<int, unsigned, float, double>,
        merge_2_packs__struct<std::tuple,
            std::tuple<int, unsigned>, std::tuple<float, double>
        >::type
    >);

    template<template<typename...> class TupleTemplate, class Tuple1, class Tuple2, class... Tuples>
    struct merge_packs__struct {
        using type = merge_packs__struct<TupleTemplate, typename merge_2_packs__struct<TupleTemplate,Tuple1,Tuple2>::type, Tuples...>::type;
    };

    template<template<typename...> class TupleTemplate, class Tuple1, class Tuple2>
    struct merge_packs__struct<TupleTemplate, Tuple1, Tuple2> {
        using type = merge_2_packs__struct<TupleTemplate,Tuple1,Tuple2>::type;
    };


    //test
    static_assert(std::same_as<
        std::tuple<int, unsigned, float, double, char, short, long>,
        merge_packs__struct<std::tuple,
            std::tuple<int, unsigned>, std::tuple<float, double>, std::tuple<char, short, long>
        >::type
    >);

    
    // turns a bunch of callables into a single overloaded callable
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>; // explicit deduction guide (not needed as of C++20)


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

    #define DETAIL__CALL_MACRO_FOR_EACH(MACRO,...) _GET_NTH_ARG(__VA_ARGS__, __FE_9, __FE_8, __FE_7, __FE_6, __FE_5, __FE_4, __FE_3, __FE_2, __FE_1, __FE_0)(MACRO, __VA_ARGS__)
}


#endif // ENGINE_UTILS_DETAIL_META_HPP
