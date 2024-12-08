#ifndef ENGINE_CONCEPTS_HPP
#define ENGINE_CONCEPTS_HPP

#include <typeinfo>
#include "meta_utils.hpp"
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
    class stateless_script;


    #define RESOURCES scene, nodetree_blueprint, stateless_script, shader, gal::texture, gal::vertex_array
    // technically not a concept, but nonetheless this is not a type supposed to be used as is, but instead it is supposed to be used
    // to define concept Resource and to be mapped to tuples of data structures of each resource type
    using resource_tuple_t = std::tuple<RESOURCES>;

    template <typename T> concept Resource = ContainedInTuple<T, resource_tuple_t>;

    //for debug reasons
    template<Resource T>
    constexpr const char* get_resource_typename() {
        if(std::same_as<T, scene>) return "scene";
        if(std::same_as<T, nodetree_blueprint>) return "nodetree_blueprint";
        if(std::same_as<T, shader>) return "shader";
        if(std::same_as<T, stateless_script>) return "script";
        if(std::same_as<T, gal::texture>) return "texture";
        if(std::same_as<T, gal::vertex_array>) return "vertex_array";
        slogga::stdout_log.info("get_resource_typename<T>() called for T with unknown name; falling back to mangled name {}", typeid(T).name());
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

    #define SPECIAL_NODE_DATA_CONTENTS null_node_data, camera, mesh, collision_shape, viewport

    using special_node_data_variant_t = std::variant<SPECIAL_NODE_DATA_CONTENTS>;

    template<typename T>
    concept SpecialNodeData = ContainedInVariant<T, special_node_data_variant_t>;
}

#endif // ENGINE_CONCEPTS_HPP
