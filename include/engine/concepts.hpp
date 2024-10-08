#ifndef ENGINE_CONCEPTS_HPP
#define ENGINE_CONCEPTS_HPP

#include <typeinfo>
#include "meta_utils.hpp"


// Resource and associated concepts
// forward declarations of resource types and declaration of Resource and PolymorphicResource concept
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

// SpecialNodeData concept
namespace engine {
    class camera;
    class mesh;
    class collision_shape;
    class viewport;
    class null_node_data;
    using special_node_data_variant_t = std::variant<null_node_data, camera, mesh, collision_shape, viewport>;

    template<typename T>
    concept SpecialNodeData = ContainedInVariant<T, special_node_data_variant_t>;
}

#endif // ENGINE_CONCEPTS_HPP
