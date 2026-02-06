#ifndef ENGINE_RESOURCES_MANAGER_HPP
#define ENGINE_RESOURCES_MANAGER_HPP

#include "engine/utils/meta.hpp"
#include "resources_manager/rc.hpp"
#include "resources_manager/weak.hpp"
#include "resources_manager/detail/resource_id.hpp"
#include "resources_manager/detail/typedefs.hpp"

//include all resource types so all their types are included whenever rc is used for that type
#include <GAL/texture.hpp>
#include <GAL/vertex_array.hpp>
#include <engine/scene/renderer/mesh/material.hpp>
#include <engine/scene/node.hpp>
#include <engine/scene.hpp>
#include <engine/utils/hash.hpp>

// resources_manager: implements shared ownership of resources and garbage collection of unused ones
// frequently abbreviated as rm to save keystrokes/screenspace since it is used everywhere

namespace engine {
    enum class internal_resource_name_t : uint8_t {
        whole_screen_vao, simple_3d_shader, dither_texture,
    };

    [[nodiscard]]
    resources_manager& get_rm();


    class resources_manager {
        template<Resource T>
        friend void flag_for_deletion(resources_manager& rm, detail::resource_id<T> resource);

        detail::resources_manager_hashmaps m_hashmaps;

        hashmap<std::string, std::function<scene()>> m_dbg_scene_ctors;
        // necessary because of the debug implementation of scene_construction
        template<Resource T>
        friend T construct_from_name(const std::string& name);

        rc<const shader> m_default_3d_shader;

        resources_manager() = default;
        ~resources_manager();

        template<Resource T> rc<T> load_impl(const detail::resource_name_t& name);
    public:
        // hand over ownership of resource to resources_manager
        template<MoveableResource T> [[nodiscard]] rc<T> new_from(T&& res);

        // inplace-construct a resource owned by resources_manager; useful for non move-constructible resources
        template<Resource T, typename... Ts> [[nodiscard]] rc<T> new_emplace(Ts... args) {
            // allocate memory
            detail::rc_ptr<T> p(new detail::rc_resource<T>(std::nullopt));
            detail::rc_resource<T>* allocation = p.get();

            // insert into hashmap id-to-resource hashmap
            detail::resource_id<T> key = *p;
            m_hashmaps.id_to_resource<T>().insert({ key, std::tuple(std::monostate(), std::move(p)) });

            //in-place construct
            allocation->resource().emplace(std::forward<Ts>(args)...);

            //make rc pointer to memory and return
            return rc<T>(allocation);
        }


        template<AnyOneOf<shader, nodetree_blueprint> T> void hot_reload(const std::string& name);

        // load resource from disk (or get it if already loaded)
        template<AnyOneOf<shader, nodetree_blueprint, gal::texture, scene> T> [[nodiscard]]
        rc<const T> load(const std::string& name) { return load_impl<T>(name); }
        // load internal resource (or get it if already loaded)
        template<AnyOneOf<shader, gal::texture, gal::vertex_array> T> [[nodiscard]]
        rc<const T> load(internal_resource_name_t name) { return load_impl<T>(uint8_t(name)); }

        // load resource from disk as mutable (build new one regardless if already loaded)
        template<AnyOneOf<shader, nodetree_blueprint, gal::texture, scene> T> [[nodiscard]]
        rc<T> load_mut(const std::string& p);

        // get/set default 3d shader (for models loaded from file). if it is null (it is by default) the "simple 3d shader" is used instead
        ///TODO: allow user to embed shader information in gltf (which would also allow different meshes in the same gltf to use different shaders)
        [[nodiscard]] rc<const shader> get_default_3d_shader();
        void set_default_3d_shader(rc<const shader> s);

        // useful for debugging scene loading behaviour since for now we don't have scenes that are loaded from file
        void dbg_add_scene_constructor(std::string name, std::function<scene()> scene_constructor);

        // destroys and deallocates resources not in use
        void collect_garbage();

        // get singleton instance
        [[nodiscard]] static resources_manager& get_instance();    private:
        friend class application;
        static void init_instance();
        static void deinit_instance();
    };


    template<Resource T>
    void flag_for_deletion(resources_manager& rm, detail::resource_id<T> resource);


}

#endif // ENGINE_RESOURCES_MANAGER_HPP
