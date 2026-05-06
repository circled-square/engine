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
#include <flat_set>

namespace engine {

    class ecs_exception;

    struct children_vector {
        std::vector<ecs_id_t> vector;
        bool is_sorted;
    };
    using components_used_set_t = std::flat_set<component_name_t>;

    class entity_component_system {
        hashmap<component_name_t, std::unique_ptr<ecs_component_interface>> m_components; // associates to each component name its implementation
        hashmap<ecs_id_t, components_used_set_t> m_components_used; // for each entity the components it uses; TODO: should this really be a vector instead of a hashmap?

        ecs_id_t m_id_pool_size = 0;

        interval_set<ecs_id_t> m_freed_ids;
    public:

        entity_component_system() {
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_dense_vector<children_vector>("children", { .is_sorted = true })));
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_dense_vector<ecs_id_t>("father", null_ecs_id)));

            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_dense_vector<glm::mat4>("transform", glm::mat4(1.))));
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_optional_hashmap<glm::mat4>("global_transform_cache")));
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_optional_hashmap<glm::mat4>("transform_edits")));
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_optional_hashmap<std::string>("name")));
        }

        void register_new_component(std::unique_ptr<ecs_component_interface> component);

        template<typename T>
        ecs_component_typed_interface<T>& get_component(component_name_t name) {
            return *dynamic_cast<ecs_component_typed_interface<T>*>(m_components[name].get());
        }

        // the components used are only really ever used for cleaning them up on id deallocation, and aren't automatically updated when a new component is used by an entity
        ecs_id_t make_new_id(components_used_set_t components_used);

        void add_new_used_component(ecs_id_t id, component_name_t component_name);

        void release_id(ecs_id_t id);

        // for profiling/debugging
        ecs_id_t get_id_pool_size() const { return m_id_pool_size; }
        ecs_id_t get_freed_ids() const { return m_freed_ids.size(); }
        ecs_id_t get_free_id_intervals_count() const { return m_freed_ids.intervals_count(); };
    };

    class component_name_already_in_use_exception : public std::exception {
        component_name_t m_component_name;
        mutable std::string m_msg_cache;
    public:
        component_name_already_in_use_exception(component_name_t name) : m_component_name(std::move(name)) {}

        const char* what() const noexcept override;
    };

    struct ran_out_of_ecs_ids_exception : public std::exception {
        const char* what() const noexcept override { return "entity component system ran out of IDs!"; }
    };

    void commit_transform_edits();
}

#endif // ENGINE_ENTITY_COMPONENT_SYSTEM_HPP
