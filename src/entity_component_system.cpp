#include <engine/entity_component_system.hpp>
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>

namespace engine {
    void entity_component_system::register_new_component(std::unique_ptr<ecs_component_interface> component) {
        if(m_components.contains(component->component_name())) {
            throw component_name_already_in_use_exception(component->component_name());
        }

        m_components.insert({component->component_name(), std::move(component)});
    }

    ecs_id_t entity_component_system::make_new_id(components_used_set_t components_used) {
        // get id
        if(m_freed_ids.empty()) {
            ecs_id_t ids_previously_in_use = m_id_pool_size;
            m_id_pool_size = std::max(m_id_pool_size * 2, m_id_pool_size + 1);
            m_freed_ids.insert_at_end({ids_previously_in_use, m_id_pool_size - 1});

            for(std::pair<component_name_t, std::unique_ptr<ecs_component_interface>>& c : m_components) {
                c.second->number_of_ids_in_use_changed(m_id_pool_size);
            }
        }
        ASSERTS(!m_freed_ids.empty());

        ecs_id_t id = m_freed_ids.extract_first_element();

        if(id == null_ecs_id) {
            throw ran_out_of_ecs_ids_exception();
        }

        // init components
        for(const auto& component_name : components_used) {
            m_components[component_name]->init_for_entity(id); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // hashmap access is safe
        }

        m_components_used.insert({id, std::move(components_used)});

        return id;
    }

    void entity_component_system::add_new_used_component(ecs_id_t id, component_name_t component_name) { m_components_used[id].insert(component_name); } // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // hashmap access is safe

    // well defined for n/0 (just returns numeric_limits::max())
    inline ecs_id_t unsigned_integer_division(ecs_id_t n, ecs_id_t d) {
        return d != 0 ? n / d : std::numeric_limits<ecs_id_t>::max();
    }

    void entity_component_system::release_id(ecs_id_t id) {
        // delete entities the component uses
        const auto& components_used = m_components_used[id]; // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // hashmap access is safe
        for(const component_name_t& component_name : components_used) {
            m_components[component_name]->uninit_for_entity(id); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access) // hashmap access is safe
        }

        // free the id for future use
        m_freed_ids.insert(id);

        auto last_freed_interval = m_freed_ids.peek_last_interval();
        if(last_freed_interval.b == this->m_id_pool_size - 1) {
            const ecs_id_t minimum_new_pool_size = last_freed_interval.a; // we cannot erase ids < to this, since last_freed_interval.a - 1 is the biggest id in use (or last_freed_interval.a is 0)
            const ecs_id_t allocated_ids = m_id_pool_size - m_freed_ids.size();
            const ecs_id_t current_inverse_load_factor = unsigned_integer_division(m_id_pool_size, allocated_ids);
            const ecs_id_t new_pool_size = allocated_ids * 2; // allocate space for twice the current number of used ids, so we don't keep growing and shrinking

            if(current_inverse_load_factor >= 4 // load factor <= 1/4
                && minimum_new_pool_size <= new_pool_size) // shrinking does not erase any id in use
            {
                m_id_pool_size = new_pool_size;
                m_freed_ids.erase_last_interval(); // erase all ids which we can erase

                if(last_freed_interval.a <= m_id_pool_size - 1) {
                    m_freed_ids.insert_at_end({last_freed_interval.a, m_id_pool_size - 1});
                }

                for(std::pair<component_name_t, std::unique_ptr<ecs_component_interface>>& c : m_components) {
                    c.second->number_of_ids_in_use_changed(m_id_pool_size);
                }

            }
        }
    }

    const char* component_name_already_in_use_exception::what() const noexcept {
        if(m_msg_cache.empty()) {
            m_msg_cache = std::format("component name already in use: \"{}\"", m_component_name);
        }
        return m_msg_cache.c_str();
    }

    void commit_transform_edits(entity_component_system ecs) {
        auto& transform_edits = dynamic_cast<ecs_component_optional_hashmap<glm::mat4>&>(ecs.get_component<glm::mat4>("transform_edits"));
        auto& transforms = ecs.get_component<glm::mat4>("transform");
        auto& cached_global_transforms = ecs.get_component<glm::mat4>("global_transform_cache");

        for(const auto&[id, v] : transform_edits.underlying_hashmap()) {
            cached_global_transforms.uninit_for_entity(id); // TODO : uninit children?
            UNIMPLEMENTED(false && "uninit children and children's children and whatnot?");
            transforms.set(id, v);
        }

        transform_edits.erase_contents();
    }
}




