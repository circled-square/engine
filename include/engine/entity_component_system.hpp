#ifndef ENGINE_ENTITY_COMPONENT_SYSTEM_HPP
#define ENGINE_ENTITY_COMPONENT_SYSTEM_HPP

#include <vector>
#include <glm/glm.hpp>
#include <engine/utils/hash.hpp>
#include <engine/utils/bounds_check_access.hpp>
#include <engine/utils/interval_set.hpp>
#include <memory>
#include <slogga/asserts.hpp>
#include <engine/entity_component_system/component_implementations.hpp>
#include <engine/scene/node/script.hpp>
#include <engine/scene/node/collision_behaviour.hpp>
#include <engine/scene/node/camera.hpp>
#include <engine/scene/node/viewport.hpp>

namespace engine {
    class ecs_exception;

    struct children_vector {
        std::vector<ecs_id_t> vector;
        bool is_sorted;
    };

    namespace component_names {
        enum : component_name_t {
            children = 0,
            father,
            collision_behaviour,
            transform,
            name,

            transform_edits,
            global_transform_cache,
            script,
            blueprint,
            camera,
            mesh,
            collision_shape,
            viewport,

            number_of_reserved_names,
            start_of_nonreserved_names = number_of_reserved_names
        };
        // gets default component for a certain type, e.g. default_component_name<children_vector>() -> children
        template<AnyOneOf<children_vector, node_collision_behaviour, engine::script, rc<const nodetree_blueprint>, engine::camera, engine::mesh, rc<const engine::collision_shape>, engine::viewport> T>
        consteval uint64_t default_component_name() {
            if constexpr(std::same_as<T, children_vector>) { return children; }
            else if constexpr(std::same_as<T, node_collision_behaviour>) { return collision_behaviour; }
            else if constexpr(std::same_as<T, engine::script>) { return script; }
            else if constexpr(std::same_as<T, rc<const nodetree_blueprint>>) { return blueprint; }
            else if constexpr(std::same_as<T, engine::camera>) { return camera; }
            else if constexpr(std::same_as<T, engine::mesh>) { return mesh; }
            else if constexpr(std::same_as<T, rc<const engine::collision_shape>>) { return collision_shape; }
            else if constexpr(std::same_as<T, engine::viewport>) { return viewport; }
        }
    }


    class entity_component_system {
        // hashmap<component_name_t, std::unique_ptr<ecs_component_interface>> m_components; // associates to each component name its implementation
        std::vector<std::unique_ptr<ecs_component_interface>> m_components; // associates to each component name its implementation

        ecs_id_t m_id_pool_size = 0;

        interval_set<ecs_id_t> m_freed_ids;
    public:

        entity_component_system() {
            namespace names = component_names;

            m_components.reserve(names::number_of_reserved_names);

            using component_ptr = std::unique_ptr<ecs_component_interface>;

            // mandatory components
            register_new_component(component_ptr(new ecs_component_dense_vector<children_vector>(names::children, { .is_sorted = true })));
            register_new_component(component_ptr(new ecs_component_dense_vector<ecs_id_t>(names::father, null_ecs_id)));
            register_new_component(component_ptr(new ecs_component_dense_vector<node_collision_behaviour>(names::collision_behaviour, {})));
            register_new_component(component_ptr(new ecs_component_dense_vector<glm::mat4>(names::transform, glm::mat4(1.))));
            register_new_component(component_ptr(new ecs_component_dense_vector<std::string>(names::name, "default_node_name")));

            // optional components
            register_new_component(component_ptr(new ecs_component_optional_hashmap<glm::mat4>(names::transform_edits)));
            register_new_component(component_ptr(new ecs_component_optional_hashmap<glm::mat4>(names::global_transform_cache)));
            register_new_component(component_ptr(new ecs_component_optional_hashmap<script>(names::script)));
            register_new_component(component_ptr(new ecs_component_optional_hashmap<rc<const nodetree_blueprint>>(names::blueprint))); // reference to the nodetree blueprint the node was built from, if any, to keep its refcount up
            register_new_component(component_ptr(new ecs_component_optional_hashmap<camera>(names::camera)));
            register_new_component(component_ptr(new ecs_component_optional_hashmap<mesh>(names::mesh)));
            register_new_component(component_ptr(new ecs_component_optional_hashmap<rc<const collision_shape>>(names::collision_shape)));
            register_new_component(component_ptr(new ecs_component_optional_hashmap<viewport>(names::viewport)));
        }
        entity_component_system(entity_component_system&&) = delete;
        entity_component_system(const entity_component_system&) = delete;
        entity_component_system& operator=(entity_component_system&&) = delete;
        entity_component_system& operator=(const entity_component_system&) = delete;
        ~entity_component_system() = default;

        void register_new_component(std::unique_ptr<ecs_component_interface> component);

        template<typename T>
        ecs_component_typed_interface<T>& get_component(component_name_t name = component_names::default_component_name<T>()) {
            return *dynamic_cast<ecs_component_typed_interface<T>*>(m_components[name].get());
        }


        ecs_id_t make_new_id();
        // create a new id and copy the values for all components, except father and children.
        ecs_id_t shallow_clone(ecs_id_t id);

        // TODO: currently we must dealloc components the id doesn't even use!
        void release_id(ecs_id_t id);

        // for profiling/debugging
        ecs_id_t get_id_pool_size() const { return m_id_pool_size; }
        ecs_id_t get_freed_ids() const { return m_freed_ids.size(); }
        ecs_id_t get_free_id_intervals_count() const { return m_freed_ids.intervals_count(); };
    };

    class invalid_component_name_exception : public std::exception {
        component_name_t m_component_name;
        mutable std::string m_msg_cache;
    public:
        invalid_component_name_exception(component_name_t name) : m_component_name(std::move(name)) {}

        const char* what() const noexcept override;
    };

    struct ran_out_of_ecs_ids_exception : public std::exception {
        const char* what() const noexcept override { return "entity component system ran out of IDs!"; }
    };

    void commit_transform_edits();
}

#endif // ENGINE_ENTITY_COMPONENT_SYSTEM_HPP
