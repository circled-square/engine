#ifndef ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP
#define ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP

#include <engine/utils/meta.hpp>
#include <variant>
#include <engine/resources_manager/rc.hpp>
#include <engine/scene/renderer/mesh.hpp>
#include "viewport.hpp"
#include "camera.hpp"
#include "narrow_phase_collision.hpp"

namespace engine {
    #define NODE_PAYLOAD_CONTENTS std::monostate, engine::camera, engine::mesh, engine::rc<const engine::collision_shape>, engine::viewport

    using node_payload_t = std::variant<NODE_PAYLOAD_CONTENTS>;

    template<typename T>
    concept NodePayload = ContainedInVariant<T, node_payload_t>;
}


#endif // ENGINE_SCENE_NODE_NODE_DATA_CONCEPT_HPP
