#ifndef ENGINE_ENTITY_COMPONENT_SYSTEM_HPP
#define ENGINE_ENTITY_COMPONENT_SYSTEM_HPP

#include <vector>
#include <glm/glm.hpp>
#include <concepts>
#include <engine/utils/hash.hpp>
#include <engine/utils/bounds_check_access.hpp>
#include <engine/utils/interval_set.hpp>
#include <memory>
#include <slogga/asserts.hpp>
#include <engine/entity_component_system/component_implementations.hpp>

namespace engine {

    class ecs_exception;

    inline void commit_transform_edits() {

    }

    class entity_component_system {
        hashmap<component_name_t, std::unique_ptr<ecs_component_interface>> m_components; // associates to each component name its implementation
        hashmap<ecs_id_t, std::vector<component_name_t>> m_components_used; // for each entity the components it uses

        ecs_id_t m_ids_in_use = 0;

        interval_set<ecs_id_t> m_freed_ids;
    public:

        entity_component_system() {
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_dense_vector<glm::mat4>("transform", glm::mat4(1.))));
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_optional_hashmap<glm::mat4>("global_transform_cache")));
            register_new_component(std::unique_ptr<ecs_component_interface>(new ecs_component_optional_hashmap<glm::mat4>("transform_edits")));
        }

        void register_new_component(std::unique_ptr<ecs_component_interface> component);

        template<typename T>
        ecs_component_typed_interface<T>& get_component(component_name_t name) {
            return *dynamic_cast<ecs_component_typed_interface<T>*>(m_components[name].get());
        }

        ecs_id_t make_new_id(const std::vector<component_name_t>& components_used);

        void release_id(ecs_id_t id);

        // for profiling/debugging
        ecs_id_t get_ids_in_use() const { return m_ids_in_use; }
        ecs_id_t get_freed_ids() const { return m_freed_ids.size(); }
        ecs_id_t get_free_id_intervals_count() const { return m_freed_ids.intervals_count(); };
    };

    class ecs_exception : public std::exception {
    public:
        enum class code_t : std::uint8_t { component_name_already_in_use };
    private:
        code_t m_code;
        component_name_t m_component_name;
        mutable std::string m_msg_cache;
    public:
        ecs_exception(code_t code, component_name_t component_name) : std::exception(), m_code(code), m_component_name(component_name) {}

        code_t get_code() const noexcept { return m_code; }

        const char* what() const noexcept override;
    };
}

#endif // ENGINE_ENTITY_COMPONENT_SYSTEM_HPP
