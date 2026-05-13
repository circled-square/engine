#ifndef ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP
#define ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP

#include <engine/utils/meta.hpp>
#include <stdexcept>
#include <format>

// This header contains Resource and associated concepts

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
    class stateless_script;
    class collision_shape;
    class mesh;
}

namespace engine {
    #define RESOURCES ::engine::scene, ::engine::nodetree_blueprint, ::engine::shader, \
        ::engine::collision_shape, ::gal::texture, ::gal::vertex_array, ::dylib::library
    #define MOVEABLE_RESOURCES ::engine::scene, ::engine::nodetree_blueprint, ::engine::shader, \
        ::engine::collision_shape, ::gal::texture, ::gal::vertex_array, ::dylib::library

    // technically not a concept, but nonetheless this is not a type supposed to be used as is, but instead it is supposed to be used
    // to define concept Resource and to be mapped to tuples of data structures of each resource type
    using resources_t = type_list<RESOURCES>;


    template <typename T> concept Resource = ContainedInTypeList<T, resources_t>;
    template <typename T> concept MoveableResource = Resource<T> && ContainedInTypeList<T, type_list<MOVEABLE_RESOURCES>>;

    //for debug reasons
    template<Resource T>
    constexpr const char* get_resource_typename() {
        if(std::same_as<T, scene>) return "scene";
        if(std::same_as<T, nodetree_blueprint>) return "nodetree_blueprint";
        if(std::same_as<T, shader>) return "shader";
        if(std::same_as<T, collision_shape>) return "collision shape";
        if(std::same_as<T, gal::texture>) return "texture";
        if(std::same_as<T, gal::vertex_array>) return "vertex array";
        if(std::same_as<T, dylib::library>) return "dynamic library";

        throw std::runtime_error(std::format("get_resource_typename<T>() called for T with unknown name, where typeid(T) = {}", typeid(T).name()));
    }
}



#endif // ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP
