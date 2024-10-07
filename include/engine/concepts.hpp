#ifndef ENGINE_CONCEPTS_HPP
#define ENGINE_CONCEPTS_HPP

#include <typeinfo>
#include <type_traits>
#include <concepts>
#include <tuple>

namespace engine {
    template<typename T>
    concept Polymorphic = std::is_polymorphic_v<T>;
    template<class T, class U>
    concept Derived = std::is_base_of<U, T>::value;


    template<typename T, typename... Ts>
    concept AnyOneOf = (std::same_as<T, Ts> || ...);

    namespace detail {
        template<typename T, typename tuple_t>
        struct contained_in_tuple__struct {
            static_assert(false, "tuple_t must be an instantiation of std::tuple");
        };
        template<typename T, typename...Ts>
        struct contained_in_tuple__struct<T, std::tuple<Ts...>> {
            static constexpr bool value = AnyOneOf<T, Ts...>;
        };
    }

    template<typename T, typename tuple_t>
    concept ContainedInTuple = detail::contained_in_tuple__struct<T, tuple_t>::value;
}


//forward declarations of resource types and declaration of Resource and PolymorphicResource concept
namespace gal {
    class texture;
    class vertex_array;
}
namespace engine {
    class shader;
    class basic_scene;
    class nodetree;

    // technically not a concept, but nonetheless this is not a type supposed to be used as is, but instead it is supposed to be used
    // to define concept Resource and to be mapped to tuples of data structures of each resource type
    using resource_tuple_t = std::tuple<basic_scene, nodetree, shader, gal::texture, gal::vertex_array>;

    template <typename T> concept Resource = ContainedInTuple<T, resource_tuple_t>;
    template <typename T> concept PolymorphicResource = Polymorphic<T> && Resource<T>;
    template <typename T> concept NonPolymorphicResource = !Polymorphic<T> && Resource<T>;

    //for debug reasons
    template<Resource T>
    constexpr const char* get_resource_typename() {
        if(std::same_as<T, gal::texture>) return "texture";
        if(std::same_as<T, gal::vertex_array>) return "vertex_array";
        if(std::same_as<T, shader>) return "shader";
        if(std::same_as<T, nodetree>) return "nodetree";
        if(std::same_as<T, basic_scene>) return "scene";
        return typeid(T).name();
    }
}

#endif // ENGINE_CONCEPTS_HPP
