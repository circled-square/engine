#ifndef ENGINE_RESOURCES_MANAGER_HPP
#define ENGINE_RESOURCES_MANAGER_HPP

#include "scene/node/script.hpp"
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

// resources_manager: implements shared ownership of resources and garbage collection of unused ones
// frequently abbreviated as rm to save keystrokes/screenspace since it is used everywhere

namespace engine {
    class resources_manager {
        template<Resource T>
        friend void flag_for_deletion(resources_manager& rm, detail::resource_id<T> resource);

        detail::id_to_resources_hashmaps m_active_resources;

        //NOTE: dither texture has name "dither_texture"; retro 3d shader has name "retro_3d_shader"; whole screen vao is "whole_screen_vao"
        detail::name_to_id_hashmaps m_resources_by_name;

        detail::id_hashsets m_marked_for_deletion;

        std::unordered_map<std::string, std::function<scene()>> m_dbg_scene_ctors;

        resources_manager() = default;
        ~resources_manager();
    public:
        template<Resource T> [[nodiscard]] weak<T> alloc() {
            detail::rc_ptr<T> p(new detail::rc_resource<T>(std::nullopt));
            weak<T> weak_ptr(p.get());
            auto& resources_hm = std::get<detail::id_to_resource_hashmap<T>>(m_active_resources);
            detail::resource_id<T> key = *p;
            resources_hm.insert({ key, std::make_pair(std::string(), std::move(p)) });

            return weak_ptr;
        }

        // hand over ownership of resource to resources_manager
        template<Resource T> [[nodiscard]] rc<const T> new_from(T&& res) {
            return new_mut_from<T>(std::forward<T&&>(res));
        }
        template<Resource T> [[nodiscard]] rc<T> new_mut_from(T&& res) {
            return new_mut_emplace<T>(std::forward<T&&>(res));
        }

        // inplace-construct a resource owned by resources_manager; useful for non move-constructible resources
        template<Resource T, typename... Ts> [[nodiscard]] rc<const T> new_emplace(Ts... args) {
            return new_mut_emplace<T>(std::forward<Ts>(args)...);
        }
        template<Resource T, typename... Ts> [[nodiscard]] rc<T> new_mut_emplace(Ts... args) {
            weak<T> allocation = alloc<T>();
            rc<T> lock = allocation.lock(); // semantically more correct to first lock it and then write into it

            allocation.m_resource->resource().emplace(std::forward<Ts>(args)...);
            return lock;
        }

//        template<Resource T> [[nodiscard]]
//        rc<const T> new_from_fn(const std::function<void(std::optional<T>&)>& emplace);
//        template<Resource T> [[nodiscard]]
//        rc<T> new_mut_from_fn(const std::function<void(std::optional<T>&)>& emplace);

        // get resource loaded from disk
        [[nodiscard]] rc<const gal::texture>        get_texture(const std::string& path);
        [[nodiscard]] rc<const nodetree_blueprint>  get_nodetree_from_gltf(const std::string& path);
        [[nodiscard]] rc<scene>                     get_scene(const std::string& path);

        // get resource generated at runtime
        [[nodiscard]] rc<const gal::vertex_array>   get_whole_screen_vao();
        [[nodiscard]] rc<const gal::texture>        get_dither_texture();
        [[nodiscard]] rc<const shader>              get_retro_3d_shader();


        // useful for debugging scene loading behaviour since for now we don't have scenes that are loaded from file
        void dbg_add_scene_constructor(std::string name, std::function<scene()> scene_constructor);

        // destroys and deallocates resources not in use
        void collect_garbage();

        // get singleton instance
        [[nodiscard]] static resources_manager& get_instance();
    private:
        friend class application;
        static void init_instance();
        static void deinit_instance();
    };

    template<Resource T>
    void flag_for_deletion(resources_manager& rm, detail::resource_id<T> resource);

    [[nodiscard]]
    resources_manager& get_rm();
}

#endif // ENGINE_RESOURCES_MANAGER_HPP
