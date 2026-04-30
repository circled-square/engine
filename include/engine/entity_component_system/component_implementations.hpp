#ifndef ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_IMPLEMENTATIONS_HPP
#define ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_IMPLEMENTATIONS_HPP

#include "component_interfaces.hpp"
#include <vector>
#include <engine/utils/hash.hpp>
#include <stdexcept>

namespace engine {
    template<std::copyable T>
    class ecs_component_dense_vector : public ecs_component_typed_interface<T> {
        component_name_t m_name;
        T m_default_value; //default value is set even for entities which do not have this component
        std::vector<T> m_vec;
    public:
        ecs_component_dense_vector(component_name_t name, T default_value) : m_name(name), m_default_value(std::move(default_value)) {}

        component_name_t component_name() const override { return m_name; }

        void init_for_entity(ecs_id_t id) override { bounds_check_access(m_vec, id) = m_default_value; }

        bool uninit_for_entity(ecs_id_t id) override { return false; } // do nothing: when a new entity is allocated here we will simply overwrite the value

        void number_of_ids_in_use_changed(ecs_id_t new_amount) override { m_vec.resize(new_amount, m_default_value); }


        const T& get(ecs_id_t id) const override { return bounds_check_access(m_vec, id); }
        const T* try_get(ecs_id_t id) const override { return &get(id); }
        const T& set(ecs_id_t id, T value) override { return (bounds_check_access(m_vec, id) = std::move(value)); }
    };

    template<typename T>
    class ecs_component_optional_hashmap : public ecs_component_typed_interface<T> {
        component_name_t m_name;
        hashmap<ecs_id_t, T> m_hashmap;
    public:
        ecs_component_optional_hashmap(component_name_t name) : m_name(name) {}

        component_name_t component_name() const override { return m_name; }

        void init_for_entity(ecs_id_t id) override {} // entities are free to not have this component

        bool uninit_for_entity(ecs_id_t id) override { return m_hashmap.erase(id) != 0; }

        void number_of_ids_in_use_changed(ecs_id_t new_amount) override {} // we don't care, this is mostly meant for vectors

        const T* try_get(ecs_id_t id) const override {
            auto it = m_hashmap.find(id);
            if(it == m_hashmap.end())
                return nullptr;
            return &it->second;
        }
        const T& get(ecs_id_t id) const override {
            // TODO: should we implement this and throw an exception on failure? maybe just an assertion is enough? i dislike just throwing like this because it calls into question whether we should prefer multiple inheritance
            throw std::runtime_error("called engine::ecs_component_optional_hashmap.get, but this method is not implemented for optional components; call try_get instead");
        }
        const T& set(ecs_id_t id, T value) override { return m_hashmap.insert({id, value}).first->second; }
    };
}

#endif // ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_IMPLEMENTATIONS_HPP
