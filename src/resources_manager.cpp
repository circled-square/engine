#include <engine/resources_manager.hpp>

#include <GAL/texture.hpp>
#include <GAL/vertex_array.hpp>
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>
#include <engine/scene/renderer.hpp>
#include <engine/scene/renderer/mesh/material.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp>
#include <engine/scene/node/gltf_loader.hpp>
#include <engine/scene.hpp>
#include <engine/scene/node/script.hpp>
#include <engine/scene/node.hpp>

namespace engine {
    using namespace detail;

    template<Resource T>
    void flag_for_deletion(resources_manager& rm, resource_id<T> id) {
        using namespace detail;

        auto& marked_for_deletion_set = std::get<id_hashset<T>>(rm.m_marked_for_deletion);
        marked_for_deletion_set.insert(id);
    }


    template<typename T> inline rc_ptr<T> make_rc_ptr(T res) { return rc_ptr<T>(new rc_resource<T>(std::move(res))); }

    template<Resource T>
    static rc_resource<T>* create_or_get_named_resource(auto& active_resources, auto& resources_by_name, auto& marked_for_deletion, const std::string& path, std::function<T(const std::string&)> resource_constructor) {
        auto& id_to_resource_hm = std::get<id_to_resource_hashmap<T>>(active_resources);
        auto& name_to_id_hm = std::get<name_to_id_hashmap<T>>(resources_by_name);
        auto& marked_for_deletion_set = std::get<detail::id_hashset<T>>(marked_for_deletion);

        //first retrieve the memory location
        rc_resource<T>* ret;
        if (auto search = name_to_id_hm.find(path); search != name_to_id_hm.end()) {
            // we already have the memory
            resource_id<T> id = search->second;

            //remove from marked from deletion status
            marked_for_deletion_set.erase(id);

            rc_ptr<T>& p = id_to_resource_hm[id].second;

            ret = p.get();
        } else {
            // we do not have the memory

            //create the owning pointer
            rc_ptr<T> p = rc_ptr<T>(new rc_resource<T>(std::nullopt));
            ret = p.get();

            //after allocating store the information in the hashmaps
            //create the resource id (for which we use the address)
            resource_id<T> id = *p;
            //associate the name to the id
            name_to_id_hm[path] = id;
            //associate the id to the name and owning pointer
            id_to_resource_hm.insert({id, std::make_pair(path, std::move(p))});
        }

        // then, if it is not already loaded, load the resource
        if(!ret->resource().has_value())
            ret->emplace(resource_constructor(path));

        return ret;
    }

    #define INSTANTIATE_RM_TEMPLATES(TYPE) \
        template void flag_for_deletion<TYPE>(resources_manager&, resource_id<TYPE>); \

    CALL_MACRO_FOR_EACH(INSTANTIATE_RM_TEMPLATES, RESOURCES)

    resources_manager::~resources_manager() {
        collect_garbage();
    }

    rc<const gal::texture> resources_manager::get_texture(const std::string& path) {
        return create_or_get_named_resource<gal::texture>(m_active_resources, m_resources_by_name, m_marked_for_deletion, path, [](const std::string& p) {
            return gal::texture(gal::image(p.c_str()));
        });
    }

    rc<const nodetree_blueprint> resources_manager::get_nodetree_from_gltf(const std::string& path) {
        return create_or_get_named_resource<nodetree_blueprint>(m_active_resources, m_resources_by_name, m_marked_for_deletion, path, [](const std::string& p) {
            return load_nodetree_from_gltf(p.c_str());
        });
    }

    rc<const gal::vertex_array> resources_manager::get_whole_screen_vao() {
        auto make_whole_screen_vao = [](){
            struct whole_screen_vao_vertex_t {
                glm::vec2 pos;
                using layout_t = decltype(gal::static_vertex_layout(pos));
            };

            static const std::array<whole_screen_vao_vertex_t, 4> whole_screen_vertices {
                whole_screen_vao_vertex_t {{-1,-1}}, {{1,-1}}, {{1,1}}, {{-1,1}}
            };
            static const std::array<glm::uvec3, 2> whole_screen_indices = {
                glm::uvec3 {0, 1, 2}, {2, 3, 0}
            };

            gal::vertex_buffer vbo(whole_screen_vertices);
            gal::index_buffer ibo(whole_screen_indices);

            return gal::vertex_array(std::move(vbo), std::move(ibo), whole_screen_vao_vertex_t::layout_t::to_vertex_layout());
        };

        return create_or_get_named_resource<gal::vertex_array>(m_active_resources, m_resources_by_name, m_marked_for_deletion, "whole_screen_vao", [&](...) {
            return make_whole_screen_vao();
        });
    }

    //temporary debug implementation, since we currently do not support loading scenes from file
    rc<scene> resources_manager::get_scene(const std::string &name) {
        return create_or_get_named_resource<scene>(m_active_resources, m_resources_by_name, m_marked_for_deletion, name, [&](const std::string& p) {
            EXPECTS(m_dbg_scene_ctors.contains(p));
            return this->m_dbg_scene_ctors[p]();
        });
    }

    void resources_manager::dbg_add_scene_constructor(std::string name, std::function<scene ()> scene_constructor) {
        using namespace detail;
        m_dbg_scene_ctors.insert({name, std::move(scene_constructor)});
    }

    rc<const gal::texture> resources_manager::get_dither_texture() {
        return create_or_get_named_resource<gal::texture>(m_active_resources, m_resources_by_name, m_marked_for_deletion, "dither_texture", [&](...) -> gal::texture {
            throw std::runtime_error("not implemented");
        });
    }

    rc<const shader> resources_manager::get_retro_3d_shader() {
        return create_or_get_named_resource<shader>(m_active_resources, m_resources_by_name, m_marked_for_deletion, "retro_3d_shader", [&](...) {
            return make_retro_3d_shader();
        });
    }

    void resources_manager::collect_garbage() {
        slogga::stdout_log("called resources_manager::collect_garbage");

        bool deleted_any;

        auto delete_all_marked = [&]<class T> (id_hashset<T>& marked_for_deletion_set) {
            auto& id_to_resource_hm = std::get<id_to_resource_hashmap<T>>(m_active_resources);
            auto& name_to_id_hm = std::get<name_to_id_hashmap<T>>(m_resources_by_name);

            size_t previous_size = id_to_resource_hm.size();

            while(!marked_for_deletion_set.empty()) {
                auto mfds_iterator = marked_for_deletion_set.begin();
                auto itrh_iterator = id_to_resource_hm.find(*mfds_iterator);
                EXPECTS(itrh_iterator != id_to_resource_hm.end());

                rc_resource<T>& rc_ref = *itrh_iterator->second.second;

                //refcount can be >0 if the obj was flagged and a weak<T> managed to lock() it before it was deleted, or if a rc<T> to it was obtained through its name
                if(rc_ref.refcount() == 0) {
                    if(rc_ref.weak_refcount() == 0) {
                        //there are neither rc<T> nor weak<T> pointing to this resource
                        //if present remove name-id correspondance
                        const std::string& name = itrh_iterator->second.first;
                        if(!name.empty()) {
                            EXPECTS(name_to_id_hm.contains(name));
                            name_to_id_hm.erase(name);
                        }

                        //delete resource
                        id_to_resource_hm.erase(itrh_iterator);
                    } else {
                        //there are still weak refs to this resource; destroy the contained value
                        rc_ref.resource().reset();
                    }
                }


                //remove from marked for deletion
                marked_for_deletion_set.erase(mfds_iterator);
                deleted_any = true;
            }

            slogga::stdout_log("cleared {} out of {} {}", previous_size - id_to_resource_hm.size(), previous_size, get_resource_typename<T>());
        };


        //actually only scenes can depend on each other, and no resource can depend on one that would get deleted before it. still leaving this behaviour for testing (and because who knows, maybe in the future this won't be the case)
        do {
            slogga::stdout_log("gc pass");
            deleted_any = false;

            std::apply([&](auto&... args) {
                ((delete_all_marked(args)), ...);
            }, this->m_marked_for_deletion);
        } while(deleted_any == true);
    }



    //resources manager singleton
    /* a static alloc in which a rm is in-place constructed was chosen instead of other possibilities because:
     * - a rm instance in static memory is constructed & destructed at unspecified moments in program startup/termination, which
     *   is definitely undesirable if we wish to make correct use of constructor and destructor;
     * - a Meyers singleton was avoided because lazily initing a rm seems like a bad idea, and because the destruction still
     *   happens at an unspecified time;
     * - a pointer to the heap requires a heap alloc/dealloc, which is a little weird, but not terrible;
     * - a optional<rm> destructs the object upon program shutdown, which will cause a crash if no opengl context is available;
     *
     * optional<rm> might be preferable if we wish to make deinit of rm mandatory, but as it currently stands deinit of rm is
     * only useful for debug and useless overhead otherwise, since rm only owns gpu resources and memory but nothing that needs
     * to be actually closed or saved before program termination.
     */

    alignas(resources_manager) static char global_rm_instance[sizeof(resources_manager)];
    static bool global_rm_instance_is_inited = false;

    resources_manager &resources_manager::get_instance() {
        EXPECTS(global_rm_instance_is_inited);
        return *reinterpret_cast<resources_manager*>(&global_rm_instance);
    }

    void resources_manager::init_instance() {
        EXPECTS(!global_rm_instance_is_inited);
        resources_manager* rm_p = reinterpret_cast<resources_manager*>(&global_rm_instance);
        new(rm_p) resources_manager();
        global_rm_instance_is_inited = true;
    }

    void resources_manager::deinit_instance() {
        get_instance().~resources_manager();
        global_rm_instance_is_inited = false;
    }

    resources_manager& get_rm() {
        return resources_manager::get_instance();
    }
}

