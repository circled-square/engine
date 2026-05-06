#ifndef ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_INTERFACES_HPP
#define ENGINE_ENTITY_COMPONENT_SYSTEM_COMPONENT_INTERFACES_HPP

#include <cstdint>
#include <string_view>
#include <limits>
#include <engine/utils/optional_ref.hpp>

namespace engine {
    using ecs_id_t = uint32_t;
    constexpr ecs_id_t null_ecs_id = std::numeric_limits<ecs_id_t>::max();
    /*
     * TODO:
     * 		make it a variant<str, u64>, with u64 used for built-in components so that they can be stored in an array
     * 		(instead of a hashmap, which would still be used for user components, though since we would have many leftover
     * 		indices we can just allow the user to define other indexed components). also this allows the use of a bitset
     *      for checking which entities are composed of which (indexed) components.
     *
     *      alternative: maybe we just want indexed components and that's it, maybe just allow the user to associate each
     *      component with a string name as well and fetch that name from a hashmap when necessary but otherwise only deal
     *      with vectors of components.
     */
    using component_name_t = std::string_view;

    class ecs_component_interface {
    public:
        virtual ~ecs_component_interface() = default;

        virtual component_name_t component_name() const = 0;

        virtual void init_for_entity(ecs_id_t id) = 0;
        virtual bool uninit_for_entity(ecs_id_t id) = 0;
        virtual void number_of_ids_in_use_changed(ecs_id_t new_amount) = 0;
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
