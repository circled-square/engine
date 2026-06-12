#ifndef ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_INTERFACES_HPP
#define ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_INTERFACES_HPP

#include <cstdint>
#include <limits>
#include <engine/utils/optional_ref.hpp>

namespace engine {
    using ecs_id_t = uint32_t;
    constexpr ecs_id_t null_ecs_id = std::numeric_limits<ecs_id_t>::max();
    using component_name_t = uint64_t;

    class ecs_component_interface {
    public:
        virtual ~ecs_component_interface() = default;

        virtual component_name_t component_name() const = 0;

        virtual void init_for_entity(ecs_id_t id) = 0;
        virtual bool uninit_for_entity(ecs_id_t id) = 0;
        virtual void number_of_ids_in_use_changed(ecs_id_t new_amount) = 0;
        virtual void copy(ecs_id_t from, ecs_id_t to) = 0;
    };

    template<typename T>
    class ecs_component_typed_interface : public ecs_component_interface {
    public:
        virtual ~ecs_component_typed_interface() = default;

        virtual T& get(ecs_id_t id) = 0;
        virtual const T& get(ecs_id_t id) const = 0;

        virtual optional_ref<T> try_get(ecs_id_t id) = 0;
        virtual optional_ref<const T> try_get(ecs_id_t id) const = 0;

        virtual const T& set(ecs_id_t id, T value) = 0;

        virtual const T& get_or(ecs_id_t id, const T& default_value) const final {
            optional_ref<const T> o = try_get(id);
            return o ? *o : default_value;
        }
    };
}

#endif // ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_INTERFACES_HPP
