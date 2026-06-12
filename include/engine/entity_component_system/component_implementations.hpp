#ifndef ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_IMPLEMENTATIONS_HPP
#define ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_IMPLEMENTATIONS_HPP

#include "component_interfaces.hpp"
#include <vector>
#include <engine/utils/hash.hpp>
#include <engine/utils/optional_ref.hpp>
#include <slogga/asserts.hpp>
#include <slogga/log.hpp>
#include <typeinfo>
#include <string>

namespace engine {
    template<std::copyable T>
    class ecs_component_dense_vector : public ecs_component_typed_interface<T> {
        component_name_t m_name;
        T m_default_value; //default value is set even for entities which do not have this component
        std::vector<T> m_vec;
    public:
        ecs_component_dense_vector(component_name_t name, T default_value) : m_name(name), m_default_value(std::move(default_value)) {}

        component_name_t component_name() const final { return m_name; }

        void init_for_entity(ecs_id_t id) final { bounds_check_access(m_vec, id) = m_default_value; }

        bool uninit_for_entity(ecs_id_t id) final { return false; } // do nothing: when a new entity is allocated here we will simply overwrite the value

        void number_of_ids_in_use_changed(ecs_id_t new_amount) final {
            // slogga::stdout_log("[{}].number_of_ids_in_use_changed({})", m_name, new_amount);
            m_vec.resize(new_amount, m_default_value);
        }

        void copy(ecs_id_t from, ecs_id_t to) final {
            m_vec[to] = m_vec[from];
        }


        T& get(ecs_id_t id) final { return bounds_check_access(m_vec, id); }
        const T& get(ecs_id_t id) const final { return bounds_check_access(m_vec, id); }
        optional_ref<const T> try_get(ecs_id_t id) const final { return optional_ref(get(id)); }
        optional_ref<T> try_get(ecs_id_t id) final { return optional_ref(get(id)); }
        const T& set(ecs_id_t id, T value) final { return (bounds_check_access(m_vec, id) = std::move(value)); }
    };

    class unfilled_component_exception : public std::exception {
        mutable std::string m_msg_cache;
        component_name_t m_component_name;
        ecs_id_t m_id;
        const char* m_type_name;
    public:
        unfilled_component_exception(component_name_t component, ecs_id_t id, const char* type_name) : m_component_name(component), m_id(id), m_type_name(std::move(type_name)) {}
        const char* what() const noexcept override {
            if(m_msg_cache.empty())
                m_msg_cache = std::format("unable to fetch component through ecs_component_interface::get/get_mut(): component=\"{}\", ecs_id=\"{}\", type_info=\"{}\"", m_component_name, m_id, m_type_name);
            return m_msg_cache.c_str();
        }
    };

    template<typename T>
    class ecs_component_optional_hashmap : public ecs_component_typed_interface<T> {
        component_name_t m_name;
        hashmap<ecs_id_t, T> m_hashmap;
    public:
        ecs_component_optional_hashmap(component_name_t name) : m_name(name) {}

        component_name_t component_name() const final { return m_name; }

        void init_for_entity(ecs_id_t id) final {} // entities are free to not have this component

        bool uninit_for_entity(ecs_id_t id) final { return m_hashmap.erase(id) != 0; }

        void number_of_ids_in_use_changed(ecs_id_t new_amount) final {} // we don't care, this is mostly meant for vectors

        void copy(ecs_id_t from, ecs_id_t to) final {
            auto it = m_hashmap.find(from);
            if(it != m_hashmap.end()) {
                m_hashmap.insert(std::pair{to, it->second});
            }
        }

        optional_ref<const T> try_get(ecs_id_t id) const final {
            auto it = m_hashmap.find(id);
            if(it == m_hashmap.end())
                return optional_ref<const T>();
            return optional_ref(it->second);
        }
        optional_ref<T> try_get(ecs_id_t id) final {
            auto it = m_hashmap.find(id);
            if(it == m_hashmap.end())
                return optional_ref<T>();
            return optional_ref(it->second);
        }


        // TODO: implement these and throw an exception on failure? maybe EXPECTS is enough? i dislike just throwing like this because it calls into question whether we should prefer multiple inheritance
        T& get(ecs_id_t id) final {
            auto opt = try_get(id);
            if(!opt) {
                throw unfilled_component_exception(m_name, id, typeid(T).name());
            }
            return *opt;
        }
        const T& get(ecs_id_t id) const final {
            auto opt = try_get(id);
            if(!opt) {
                throw unfilled_component_exception(m_name, id, typeid(T).name());
            }
            return *opt;
        }
        const T& set(ecs_id_t id, T value) final { return m_hashmap.insert(std::pair{id, value}).first->second; }

        // special behaviour for this specific implementation
        const hashmap<ecs_id_t, T>& underlying_hashmap() const { return m_hashmap; }
        void erase_contents() { m_hashmap.clear(); }
    };
}

#endif // ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_IMPLEMENTATIONS_HPP
