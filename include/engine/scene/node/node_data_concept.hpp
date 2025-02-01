#ifndef ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP
#define ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP

#include <engine/utils/meta.hpp>

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


#endif // ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP
