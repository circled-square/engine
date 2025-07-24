#ifndef ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP
#define ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP

#include <engine/utils/meta.hpp>
#include <variant>
#include <engine/resources_manager/rc.hpp>
#include <engine/scene/renderer/mesh.hpp>
#include "viewport.hpp"
#include "camera.hpp"
#include "narrow_phase_collision.hpp"

// SpecialNodeData concept
namespace engine {
    #define NODE_DATA_CONTENTS std::monostate, camera, mesh, rc<const collision_shape>, viewport

    using node_data_variant_t = std::variant<NODE_DATA_CONTENTS>;

    template<typename T>
    concept NodeData = ContainedInVariant<T, node_data_variant_t>;
}


#endif // ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP
