#ifndef ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP
#define ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP

#include <engine/utils/meta.hpp>
#include <stdexcept>
#include <format>

// Resource and associated concepts

// forward declarations of resource types and declaration of Resource and MoveableResource concept
namespace dylib {
    class library;
}
namespace gal {
    class texture;
    class vertex_array;
}
namespace engine {
    class shader;
    class scene;
    class nodetree_blueprint;
    class node_data;
    class stateless_script;
    class collision_shape;
}

namespace engine {
    #define RESOURCES scene, nodetree_blueprint, node_data, shader, gal::texture, gal::vertex_array, collision_shape, dylib::library
    #define MOVEABLE_RESOURCES scene, nodetree_blueprint, shader, gal::texture, gal::vertex_array, collision_shape, dylib::library

    // technically not a concept, but nonetheless this is not a type supposed to be used as is, but instead it is supposed to be used
    // to define concept Resource and to be mapped to tuples of data structures of each resource type
    using resource_tuple_t = std::tuple<RESOURCES>;


    template <typename T> concept Resource = ContainedInTuple<T, resource_tuple_t>;
    template <typename T> concept MoveableResource = Resource<T> && ContainedInTuple<T, std::tuple<MOVEABLE_RESOURCES>>;

    //for debug reasons
    template<Resource T>
    constexpr const char* get_resource_typename() {
        if(std::same_as<T, scene>) return "scene";
        if(std::same_as<T, nodetree_blueprint>) return "nodetree blueprint";
        if(std::same_as<T, node_data>) return "node";
        if(std::same_as<T, shader>) return "shader";
        if(std::same_as<T, gal::texture>) return "texture";
        if(std::same_as<T, gal::vertex_array>) return "vertex array";
        if(std::same_as<T, collision_shape>) return "collision shape";
        if(std::same_as<T, dylib::library>) return "dynamic library";

        throw std::runtime_error(std::format("get_resource_typename<T>() called for T with unknown name, where typeid(T) = {}", typeid(T).name()));
    }
}



#endif // ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP
