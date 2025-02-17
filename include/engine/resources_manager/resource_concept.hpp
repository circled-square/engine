#ifndef ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP
#define ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP

#include <engine/utils/meta.hpp>
#include <slogga/log.hpp>

// Resource and associated concepts
// forward declarations of resource types and declaration of Resource and PolymorphicResource concept
namespace gal {
    class texture;
    class vertex_array;
}
namespace engine {
    class shader;
    class scene;
    class nodetree_blueprint;
    class node;
    class stateless_script;
    class collision_shape;


    #define RESOURCES scene, nodetree_blueprint, node, stateless_script, shader, gal::texture, gal::vertex_array, collision_shape
    // technically not a concept, but nonetheless this is not a type supposed to be used as is, but instead it is supposed to be used
    // to define concept Resource and to be mapped to tuples of data structures of each resource type
    using resource_tuple_t = std::tuple<RESOURCES>;

    template <typename T> concept Resource = ContainedInTuple<T, resource_tuple_t>;

    //for debug reasons
    template<Resource T>
    constexpr const char* get_resource_typename() {
        if(std::same_as<T, scene>) return "scene";
        if(std::same_as<T, nodetree_blueprint>) return "nodetree_blueprint";
        if(std::same_as<T, node>) return "node";
        if(std::same_as<T, shader>) return "shader";
        if(std::same_as<T, stateless_script>) return "script";
        if(std::same_as<T, gal::texture>) return "texture";
        if(std::same_as<T, gal::vertex_array>) return "vertex_array";
        if(std::same_as<T, collision_shape>) return "collision_shape";
        slogga::stdout_log.info("get_resource_typename<T>() called for T with unknown name; falling back to mangled name {}", typeid(T).name());
        return typeid(T).name();
    }
}



#endif // ENGINE_RESOURCES_MANAGER_RESOURCE_CONCEPT_HPP
