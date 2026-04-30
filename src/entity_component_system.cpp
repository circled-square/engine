#include <engine/entity_component_system.hpp>
#include <slogga/log.hpp>

namespace engine {
    void entity_component_system::register_new_component(std::unique_ptr<ecs_component_interface> component) {
        if(m_components.contains(component->component_name())) {
            throw ecs_exception(ecs_exception::code_t::component_name_already_in_use, component->component_name());
        }

        m_components.insert({component->component_name(), std::move(component)});
    }

    ecs_id_t entity_component_system::make_new_id(const std::vector<component_name_t>& components_used) {
        // get id
        if(m_freed_ids.empty()) {
            ecs_id_t ids_previously_in_use = m_ids_in_use;
            m_ids_in_use = std::max(m_ids_in_use * 2, m_ids_in_use + 1);
            m_freed_ids.insert_at_end({ids_previously_in_use, m_ids_in_use - 1});

            for(std::pair<component_name_t, std::unique_ptr<ecs_component_interface>>& c : m_components) {
                c.second->number_of_ids_in_use_changed(m_ids_in_use);
            }
        }
        ASSERTS(!m_freed_ids.empty());

        ecs_id_t id = m_freed_ids.extract_first_element();

        // init components
        for(const auto& component_name : components_used) {
            m_components[component_name]->init_for_entity(id);
        }

        m_components_used.insert({id, std::move(components_used)});

        return id;
    }

    void entity_component_system::release_id(ecs_id_t id) {
        // delete id components
        const std::vector<component_name_t>& components_used = m_components_used[id]; // for each entity the components it uses
        for(const component_name_t& component_name : components_used) {
            m_components[component_name]->uninit_for_entity(id);
        }

        // free the id for future use
        m_freed_ids.insert(id);

        auto last_freed_interval = m_freed_ids.peek_last_interval();
        if(last_freed_interval.b == this->m_ids_in_use - 1) {
            ecs_id_t ids_we_cannot_erase = last_freed_interval.a;
            ecs_id_t current_inverse_load_factor = m_ids_in_use / (m_ids_in_use - m_freed_ids.size());

            if(current_inverse_load_factor >= 4
                && ids_we_cannot_erase <= m_ids_in_use / 2)
            {
                m_ids_in_use = (m_ids_in_use - m_freed_ids.size()) * 2; // allocate space for twice the current number of used ids, so we don't keep growing and shrinking
                m_freed_ids.erase_last_interval(); // erase all ids which we can erase

                if(last_freed_interval.a <= m_ids_in_use - 1) {
                    m_freed_ids.insert_at_end({last_freed_interval.a, m_ids_in_use - 1});
                }
            }
        }
    }

    const char* ecs_exception::what() const noexcept {
        if(m_msg_cache.empty()) {
            switch(m_code) {
            case code_t::component_name_already_in_use:
                m_msg_cache = std::format("component name already in use: \"{}\"", m_component_name);
                break;
            default:
                m_msg_cache = std::format("ecs_exception constructed with invalid code {} for component \"{}\"", (std::uint8_t)m_code, m_component_name);
            }
        }
        return m_msg_cache.c_str();
    }

}