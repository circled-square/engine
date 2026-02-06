#include "engine/resources_manager/detail/typedefs.hpp"
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

    // this function cannot become a member function of resources_manager, because it must be used by rc_resource which only has access to rm as an incomplete type.
    template<Resource T>
    void flag_for_deletion(resources_manager& rm, resource_id<T> id) {
        rm.m_hashmaps.marked_for_deletion<T>().insert(id);
    }


    template<Resource T>
    T construct_from_name(const std::string& p) { throw std::runtime_error("unimplemented!"); }
    template<> shader construct_from_name<shader>(const std::string& p) { return shader::from_file("assets/" + p); }
    template<> nodetree_blueprint construct_from_name<nodetree_blueprint>(const std::string& p) {
        std::string path = "assets/" + p;
        return load_nodetree_from_gltf(path.c_str(), get_rm().get_default_3d_shader());
    }
    template<> gal::texture construct_from_name<gal::texture>(const std::string& p) {
        std::string path = "assets/" + p;
        return gal::texture(gal::image(path.c_str()));
    }
    //temporary debug implementation, since we currently do not support loading scenes from file
    template<> scene construct_from_name<scene>(const std::string& name) {
        EXPECTS(get_rm().m_dbg_scene_ctors.contains(name));
        return get_rm().m_dbg_scene_ctors[name]();
    }

    template<Resource T>
    T construct_from_name(internal_resource_name_t n) { throw std::runtime_error("unimplemented!"); }
    template<> shader construct_from_name(internal_resource_name_t n) {
        if (n == internal_resource_name_t::simple_3d_shader) {
            return make_simple_3d_shader();
        } else {
            throw std::runtime_error(std::format("incorrect internal_resource_name_t passed to resources_manager::construct_from_name<shader>(): {}", uint8_t(n)));
        }
    }
    template<> gal::texture construct_from_name(internal_resource_name_t n) {
        if (n == internal_resource_name_t::dither_texture) {
            // obtained procedurally
            const std::array<std::array<std::uint8_t, 16>, 16> dither_pattern = {
                0,      128,    32,     160,    8,      136,    40,     168,    2,      130,    34,     162,    10,     138,    42,     170,
                192,    64,     224,    96,     200,    72,     232,    104,    194,    66,     226,    98,     202,    74,     234,    106,
                48,     176,    16,     144,    56,     184,    24,     152,    50,     178,    18,     146,    58,     186,    26,     154,
                240,    112,    208,    80,     248,    120,    216,    88,     242,    114,    210,    82,     250,    122,    218,    90,
                12,     140,    44,     172,    4,      132,    36,     164,    14,     142,    46,     174,    6,      134,    38,     166,
                204,    76,     236,    108,    196,    68,     228,    100,    206,    78,     238,    110,    198,    70,     230,    102,
                60,     188,    28,     156,    52,     180,    20,     148,    62,     190,    30,     158,    54,     182,    22,     150,
                252,    124,    220,    92,     244,    116,    212,    84,     254,    126,    222,    94,     246,    118,    214,    86,
                3,      131,    35,     163,    11,     139,    43,     171,    1,      129,    33,     161,    9,      137,    41,     169,
                195,    67,     227,    99,     203,    75,     235,    107,    193,    65,     225,    97,     201,    73,     233,    105,
                51,     179,    19,     147,    59,     187,    27,     155,    49,     177,    17,     145,    57,     185,    25,     153,
                243,    115,    211,    83,     251,    123,    219,    91,     241,    113,    209,    81,     249,    121,    217,    89,
                15,     143,    47,     175,    7,      135,    39,     167,    13,     141,    45,     173,    5,      133,    37,     165,
                207,    79,     239,    111,    199,    71,     231,    103,    205,    77,     237,    109,    197,    69,     229,    101,
                63,     191,    31,     159,    55,     183,    23,     151,    61,     189,    29,     157,    53,     181,    21,     149,
                255,    127,    223,    95,     247,    119,    215,    87,     253,    125,    221,    93,     245,    117,    213,    85,
            };
            static_assert(dither_pattern.size() == dither_pattern[0].size());

            gal::texture::specification spec {
                .res = {dither_pattern.size(), dither_pattern.size()},
                .components = 1,
                .data = dither_pattern.data(),
                .alignment = 1,
                .enable_mipmaps = false,
                .repeat_wrap = true,
            };

            return gal::texture(spec);
        } else {
            throw std::runtime_error(std::format("incorrect internal_resource_name_t passed to resources_manager::construct_from_name<gal::texture>(): {}", uint8_t(n)));
        }
    }
    template<> gal::vertex_array construct_from_name(internal_resource_name_t n) {
        if (n == internal_resource_name_t::whole_screen_vao) {
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
        } else {
            throw std::runtime_error(std::format("incorrect internal_resource_name_t passed to resources_manager::construct_from_name<gal::vertex_array>(): {}", uint8_t(n)));
        }
    }



    template<Resource T>
    rc<T> resources_manager::load_impl(const detail::resource_name_t& name) {
        //first retrieve the memory location
        detail::rc_resource<T>* ret;
        if (auto search = m_hashmaps.name_to_id<T>().find(name); search != m_hashmaps.name_to_id<T>().end()) {
            // we already have the memory
            detail::resource_id<T> id = search->second;

            //remove from marked from deletion status
            m_hashmaps.marked_for_deletion<T>().erase(id);

            detail::rc_ptr<T>& p = std::get<detail::rc_ptr<T>>(m_hashmaps.id_to_resource<T>()[id]);

            ret = p.get();
        } else {
            // we do not have the memory

            //create the owning pointer
            detail::rc_ptr<T> p(new detail::rc_resource<T>(std::nullopt));
            ret = p.get();

            //after allocating store the information in the hashmaps
            //create the resource id (for which we use the address)
            detail::resource_id<T> id = *p;
            //associate the name to the id
            m_hashmaps.name_to_id<T>()[name] = id;
            //associate the id to the name and owning pointer
            m_hashmaps.id_to_resource<T>().insert({id, std::tuple(name, std::move(p))});
        }

        // then, if it is not already loaded, load the resource
        if(!ret->resource().has_value()) {
            match_variant(name,
                [&](const std::string& n) { ret->emplace(construct_from_name<T>(n)); },
                [&](std::uint8_t n) { ret->emplace(construct_from_name<T>(internal_resource_name_t(n))); },
                [](std::monostate) {}
            );
        }

        return rc<T>(ret);
    }
    template<MoveableResource T> rc<T> resources_manager::new_from(T&& res) {
        return new_emplace<T>(std::forward<T&&>(res));
    }


    template<AnyOneOf<shader, nodetree_blueprint, gal::texture, scene> T>
    rc<T> resources_manager::load_mut(const std::string& p) {
        return new_from(construct_from_name<T>(p));
    }
    template rc<shader> resources_manager::load_mut<shader>(const std::string& p);
    template rc<nodetree_blueprint> resources_manager::load_mut<nodetree_blueprint>(const std::string& p);
    template rc<gal::texture> resources_manager::load_mut<gal::texture>(const std::string& p);
    template rc<scene> resources_manager::load_mut<scene>(const std::string& p);



    template<AnyOneOf<shader, nodetree_blueprint> T> void resources_manager::hot_reload(const std::string& identifier) {
        //first retrieve the memory location
        if (auto search = m_hashmaps.name_to_id<T>().find(identifier); search != m_hashmaps.name_to_id<T>().end()) {
            // we already have the memory
            detail::resource_id<T> id = search->second;

            //remove from marked from deletion status
            m_hashmaps.marked_for_deletion<T>().erase(id);

            detail::rc_ptr<T>& p = std::get<detail::rc_ptr<T>>(m_hashmaps.id_to_resource<T>()[id]);

            detail::rc_resource<T>* mem_location = p.get();

            // then, if it is already loaded, unload the resource
            mem_location->resource() = std::nullopt;

            // finally load the resource
            // ret->emplace(resource_constructor(path));
            rc<const T> ignore = load<T>(identifier);
        }
    }

    // not implementable for all resource types
    template void resources_manager::hot_reload<shader>(const std::string&);
    template void resources_manager::hot_reload<nodetree_blueprint>(const std::string&);

    #define INSTANTIATE_RM_TEMPLATES(TYPE) \
        template void flag_for_deletion<TYPE>(resources_manager&, resource_id<TYPE>); \

    CALL_MACRO_FOR_EACH(INSTANTIATE_RM_TEMPLATES, RESOURCES)

    #undef INSTANTIATE_RM_TEMPLATES

    #define INSTANTIATE_RM_TEMPLATES_MOVEABLE(TYPE) \
        template rc<TYPE> resources_manager::new_from(TYPE&& res); \
        template rc<TYPE> resources_manager::load_impl<TYPE>(const detail::resource_name_t&); \


    CALL_MACRO_FOR_EACH(INSTANTIATE_RM_TEMPLATES_MOVEABLE, MOVEABLE_RESOURCES)

    #undef INSTANTIATE_RM_TEMPLATES_MOVEABLE


    resources_manager::~resources_manager() {
        set_default_3d_shader(nullptr);
        collect_garbage();
    }

    void resources_manager::dbg_add_scene_constructor(std::string name, std::function<scene ()> scene_constructor) {
        using namespace detail;
        m_dbg_scene_ctors.insert({name, std::move(scene_constructor)});
    }

    rc<const shader> resources_manager::get_default_3d_shader() { return m_default_3d_shader ? m_default_3d_shader : load<shader>(internal_resource_name_t::simple_3d_shader); }
    void resources_manager::set_default_3d_shader(rc<const shader> s) { m_default_3d_shader = std::move(s); }

    void resources_manager::collect_garbage() {
        slogga::stdout_log("called resources_manager::collect_garbage");

        bool deleted_any;

        auto delete_all_marked = [&]<class T> (id_hashset<T>& marked_for_deletion_set) {

            std::size_t previous_size = m_hashmaps.id_to_resource<T>().size();

            while(!marked_for_deletion_set.empty()) {
                auto mfds_iterator = marked_for_deletion_set.cbegin();
                auto itrh_iterator = m_hashmaps.id_to_resource<T>().find(*mfds_iterator);
                EXPECTS(itrh_iterator != m_hashmaps.id_to_resource<T>().end());
                EXPECTS(mfds_iterator != marked_for_deletion_set.cend());

                //IMPORTANT: have care to call erase(mfds_it) BEFORE ~rc_resource() (itrh.erase, rc_resource.resource().reset()), since doing that may cause a rc_resource to reach refcount 0 and be marked for deletion; this in turn will invalidate the mfds_iterator and that is BAD (program hangs forever when trying to erase)
                marked_for_deletion_set.erase(mfds_iterator);

                rc_resource<T>& rc_ref = *std::get<rc_ptr<T>>(itrh_iterator->second);

                //refcount can be >0 if the obj was flagged and a weak<T> managed to lock() it before it was deleted, or if a rc<T> to it was obtained through its name
                if(rc_ref.refcount() == 0) {
                    if(rc_ref.weak_refcount() == 0) {
                        //there are neither rc<T> nor weak<T> pointing to this resource
                        //if present remove name-id correspondence
                        resource_name_t name = std::get<resource_name_t>(itrh_iterator->second);

                        if(!std::holds_alternative<std::monostate>(name)) {
                            EXPECTS(m_hashmaps.name_to_id<T>().contains(name));
                            m_hashmaps.name_to_id<T>().erase(name);
                        }


                        //delete resource
                        m_hashmaps.id_to_resource<T>().erase(itrh_iterator);
                    } else {
                        //there are still weak refs to this resource; destroy the contained value
                        rc_ref.resource().reset();
                    }

                    deleted_any = true;
                }
            }

            std::size_t cleared = previous_size - m_hashmaps.id_to_resource<T>().size();
            if (cleared != 0 || previous_size != 0) {
                slogga::stdout_log("cleared {} out of {} {}", cleared, previous_size, get_resource_typename<T>());
            }
        };


        //actually only scenes can depend on each other, and no resource can depend on one that would get deleted before it. still leaving this behaviour for testing (and because who knows, maybe in the future this won't be the case)
        do {
            slogga::stdout_log("gc pass");
            deleted_any = false;

            std::apply([&](auto&... args) {
                ((delete_all_marked(args)), ...);
            }, m_hashmaps.all_marked_for_deletion_sets());
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

