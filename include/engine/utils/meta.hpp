#ifndef ENGINE_UTILS_META_HPP
#define ENGINE_UTILS_META_HPP

#include <concepts>
#include <tuple>
#include <variant>
#include <optional>
#include "detail/meta.hpp"

// general purpose concepts and metaprogramming utilities
namespace engine {
    // T is AnyOneOf<Ts...> if T is contained in Ts 
    template<typename T, typename... Ts>
    concept AnyOneOf = (std::same_as<T, Ts> || ...);

    // T is ContainedInTuple<tuple_t> if tuple_t === std::tuple<..., T, ...>
    template<typename T, typename tuple_t>
    concept ContainedInTuple = detail::contained_in_pack__struct<T, std::tuple, tuple_t>::value;

    // T is ContainedInVariant<variant_t> if variant_t === std::variant<..., T, ...>
    template<typename T, typename variant_t>
    concept ContainedInVariant = detail::contained_in_pack__struct<T, std::variant, variant_t>::value;

    //takes a Tuple = InTuple<Ts...> type and returns OutTuple<Template<Ts>...> type
    template<template<class> class Template, template<class...> class InTuple, template<class...> class OutTuple, class Tuple>
    using map_pack = detail::map_pack__struct<Template, InTuple, OutTuple, Tuple>::type;

    //takes a Tuple=std::tuple<Ts...> type and returns std::tuple<Template<Ts>...>
    template<template<class> class Template, class Tuple>
    using map_tuple = map_pack<Template, std::tuple, std::tuple, Tuple>;

    //takes Tuples of the form Tuple(i)=TupleTemplate<Args(i)...> and returns TupleTemplate<Args(0)...,Args(1)..., etc>
    template<template<typename...> class TupleTemplate, class...Tuples>
    using merge_packs = detail::merge_packs__struct<TupleTemplate, Tuples...>::type;

    template<class...Variants>
    using merge_variants = merge_packs<std::variant, Variants...>;

    //like std::visit but with multiple overloads; will cause a compile error if not all of the variant's cases are covered
    template<typename...Ts>
    void match_variant(const std::variant<Ts...>& v, const auto&... handlers) {
        std::visit(detail::overloaded(handlers...), v);
    }

    template<typename func_t, typename signature>
    concept Callable = detail::impl_callable_s<func_t, signature>::value;


    // if option has value, call handler on the value
    template<typename T, Callable<void (T&)> handler_t>
    void visit_optional(std::optional<T>& o, const handler_t& handler) {
        if(o.has_value()) {
            handler(o.value());
        }
    }


    // CALL_MACRO_FOR_EACH: a for each implemented in the preprocessor
    #define CALL_MACRO_FOR_EACH(MACRO, ...) DETAIL__CALL_MACRO_FOR_EACH(MACRO, __VA_ARGS__)
}
#endif // ENGINE_UTILS_META_HPP
