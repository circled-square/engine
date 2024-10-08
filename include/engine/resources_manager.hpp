#ifndef ENGINE_RESOURCES_MANAGER_HPP
#define ENGINE_RESOURCES_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include "rc.hpp"

#include <GAL/texture.hpp>
#include <GAL/vertex_array.hpp>
#include "material.hpp"
#include "scene/node.hpp"
#include "scene/basic_scene.hpp"
#include <functional>

namespace engine {
    namespace detail {
        // An owning pointer with reference counting, can be used to construct a rc
        template<Resource T> using rc_ptr = std::unique_ptr<rc_resource<T>>;


        // The id of a resource, which can be used to retrieve from rm's hashmaps the resource itself, its name and whether it is flagged for deletion.
        // To be unique to the resource, tied to its type, for std::hash and for simplicity a ptr could be used; however a resource_id is not meant to be derefed or used in math, so this was deemed inappropriate
        template<Resource T>
        class resource_id {
            std::uintptr_t m_id; // if m_id is 0 the id is null
            friend class resource_id_hash;
        public:
            resource_id() : m_id(0) {}
            resource_id(const rc_resource<T>& resource) : m_id((std::uintptr_t)&resource) {}
            resource_id(const resource_id& o) : m_id(o.m_id) {}
            bool operator==(const resource_id& o) const { return m_id == o.m_id; }
        };

        struct resource_id_hash {
            std::size_t operator()(const auto& rid) const noexcept {
                return std::hash<std::uintptr_t>{}(rid.m_id);
            };
        };


        // these aliases are necessary to compose complex types from resource_tuple_t with map_tuple

        // map from id to (name, resource)
        template<Resource T> using id_to_resource_hashmap = std::unordered_map<resource_id<T>, std::pair<std::string, rc_ptr<T>>, resource_id_hash>;
        // map from name to id
        template<Resource T> using name_to_id_hashmap = std::unordered_map<std::string, resource_id<T>>;
        // set of ids for GCion
        template<Resource T> using deletion_set = std::unordered_set<resource_id<T>, resource_id_hash>;
    }

    // the class resources_manager implements shared ownership of resources and garbage collection of unused ones
    // frequently abbreviated as rm
    class resources_manager {
        template<Resource T>
        friend void flag_for_deletion(resources_manager& rm, rc_resource<T>* resource);

        template<Resource T> friend class rm_templates_instantiator;

        map_tuple<detail::id_to_resource_hashmap, resource_tuple_t> m_active_resources;

        //NOTE: dither texture has name "dither_texture"; retro 3d shader has name "retro_3d_shader"
        map_tuple<detail::name_to_id_hashmap, resource_tuple_t> m_resources_by_name;

        map_tuple<detail::deletion_set, resource_tuple_t> m_marked_for_deletion;

        std::unordered_map<std::string, std::function<detail::rc_ptr<basic_scene>()>> m_dbg_scene_ctors;

        // TODO: make this something other than a private method, since private methods of this class are supposed to be used by engine::application
        template<Resource T> rc<T> create_or_get_named_resource(const std::string& path, std::function<detail::rc_ptr<T>(const std::string&)> resource_constructor);

        resources_manager() = default;
        ~resources_manager() = default;
    public:

        template <NonPolymorphicResource T> [[nodiscard, maybe_unused]]
        rc<const T> new_from(T&& res);
        template <PolymorphicResource base_t, Derived<base_t> derived_t> [[nodiscard, maybe_unused]]
        rc<const base_t> new_from(derived_t&& res) {
            using namespace detail;

            auto p = rc_ptr<base_t>(new rc_resource_derived<base_t, derived_t>(std::move(res)));
            rc<const base_t> ret(p.get());
            auto& resources_hm = std::get<id_to_resource_hashmap<base_t>>(m_active_resources);
            resource_id<base_t> key = *p;
            resources_hm.insert({ key, std::make_pair(std::string(), std::move(p)) });
            return ret;
        }

        template <NonPolymorphicResource T> [[nodiscard, maybe_unused]]
        rc<T> new_mut_from(T&& res);
        template <PolymorphicResource base_t, Derived<base_t> derived_t> [[nodiscard, maybe_unused]]
        rc<base_t> new_mut_from(derived_t&& res) {
            using namespace detail;

            auto p = rc_ptr<base_t>(new rc_resource_derived<base_t, derived_t>(std::move(res)));
            rc<base_t> ret(p.get());
            auto& resources_hm = std::get<id_to_resource_hashmap<base_t>>(m_active_resources);
            resource_id<base_t> key = *p;
            resources_hm.insert({ key, std::make_pair(std::string(), std::move(p)) });
            return ret;
        }

        [[nodiscard, maybe_unused]]
        rc<const gal::texture> get_texture(const std::string& path);

        [[nodiscard, maybe_unused]]
        rc<const nodetree> get_nodetree_from_gltf(const std::string& path);

        [[nodiscard, maybe_unused]]
        rc<const gal::vertex_array> get_whole_screen_vao();

        [[nodiscard, maybe_unused]]
        rc<basic_scene> get_scene(const std::string& name);

        //useful for debugging scene loading behaviour since for now we don't have scenes that are loaded from file
        template<Derived<basic_scene> scene_t> [[maybe_unused]]
        void dbg_add_scene_constructor(std::string name, std::function<scene_t()> scene_constructor) {
            using namespace detail;
            std::function<rc_ptr<basic_scene>()> rc_scene_constructor = [scene_ctor = std::move(scene_constructor)]() {
                return rc_ptr<basic_scene>(new rc_resource_derived<basic_scene, scene_t>(scene_ctor()));
            };
            m_dbg_scene_ctors.insert({name, std::move(rc_scene_constructor)});
        }

        [[nodiscard, maybe_unused]]
        rc<const gal::texture> get_dither_texture();

        [[nodiscard, maybe_unused]]
        rc<const shader> get_retro_3d_shader();

        void collect_garbage();

        [[nodiscard]]
        static resources_manager& get_instance();
    private:
        friend class application;
        static void init_instance();
        static void deinit_instance();
    };

    template<Resource T>
    void flag_for_deletion(resources_manager& rm, rc_resource<T>* resource);


    [[nodiscard]]
    resources_manager& get_rm();
}

#endif // ENGINE_RESOURCES_MANAGER_HPP
