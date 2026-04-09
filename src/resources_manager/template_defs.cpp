#include <engine/resources_manager.hpp>
#include <slogga/log.hpp>
#include <engine/scene/renderer/mesh/material/materials.hpp> // make_simple_3d_shader
#include <engine/scene/node/gltf_loader.hpp>
#include <GAL/texture.hpp>
#include <engine/scene.hpp>
#include <dylib.hpp>
#include <engine/scene/yaml_loader.hpp>

namespace engine {

    // this function cannot become a member function of resources_manager, because it must be used by rc_resource which only has access to rm as an incomplete type.
    template<Resource T>
    void flag_for_deletion(resources_manager& rm, detail::resource_id<T> id) {
        rm.m_hashmaps.marked_for_deletion<T>().insert(id);
    }


    template<Resource T>
    T construct_from_name(std::string_view p) { throw std::runtime_error("unimplemented!"); }
    template<> shader construct_from_name<shader>(std::string_view p) { return shader::from_file(std::format("assets/{}", p)); }
    template<> nodetree_blueprint construct_from_name<nodetree_blueprint>(std::string_view p) {
        std::string path = std::format("assets/{}", p);
        return load_nodetree_from_gltf(path.c_str(), get_rm().get_default_3d_shader());
    }
    template<> gal::texture construct_from_name<gal::texture>(std::string_view p) {
        std::string path = std::format("assets/{}", p);
        return gal::texture(gal::image(path.c_str()));
    }
    //temporary debug implementation, since we currently do not support loading scenes from file
    template<> scene construct_from_name<scene>(std::string_view name) {
        std::string name_string(name);
        if(get_rm().get_dbg_scene_ctors().contains(name_string)) {
            return get_rm().get_dbg_scene_ctors().at(name_string)();
        } else {
            std::string path_string = std::format("assets/{}", name);
            return load_scene_from_yaml(path_string.c_str());
        }
    }
    template<> dylib::library construct_from_name<dylib::library>(std::string_view name) {
        dylib::decorations only_extension = dylib::decorations::os_default();
        only_extension.prefix = "";
        std::string path = std::format("assets/{}", name);
#ifndef NDEBUG
        dylib::decorations debug_prefix = only_extension;
        debug_prefix.prefix = "dbgsym_";
        try {
            return dylib::library(path, debug_prefix);
        } catch(std::exception&) {
            slogga::stdout_log.warn("failed to load shared library {} with prefix '{}', despite engine being built in debug mode; debug symbols for this shared library will not be available", name, debug_prefix.prefix);
        }
#endif

        return dylib::library(path, only_extension);
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
                [&](std::string_view n) { ret->emplace(construct_from_name<T>(n)); },
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
    rc<T> resources_manager::load_mut(std::string_view p) {
        return new_from(construct_from_name<T>(p));
    }

    template<AnyOneOf<shader, nodetree_blueprint> T> void resources_manager::hot_reload(std::string_view identifier) {
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
    template void resources_manager::hot_reload<shader>(std::string_view);
    template void resources_manager::hot_reload<nodetree_blueprint>(std::string_view);

    template rc<shader> resources_manager::load_mut<shader>(std::string_view p);
    template rc<nodetree_blueprint> resources_manager::load_mut<nodetree_blueprint>(std::string_view p);
    template rc<gal::texture> resources_manager::load_mut<gal::texture>(std::string_view p);
    template rc<scene> resources_manager::load_mut<scene>(std::string_view p);

    #define INSTANTIATE_RM_TEMPLATES(TYPE) \
        template void flag_for_deletion<TYPE>(resources_manager&, detail::resource_id<TYPE>); \

    CALL_MACRO_FOR_EACH(INSTANTIATE_RM_TEMPLATES, RESOURCES)

    #undef INSTANTIATE_RM_TEMPLATES

    #define INSTANTIATE_RM_TEMPLATES_MOVEABLE(TYPE) \
        template rc<TYPE> resources_manager::new_from(TYPE&& res); \
        template rc<TYPE> resources_manager::load_impl<TYPE>(const detail::resource_name_t&); \


    CALL_MACRO_FOR_EACH(INSTANTIATE_RM_TEMPLATES_MOVEABLE, MOVEABLE_RESOURCES)

    #undef INSTANTIATE_RM_TEMPLATES_MOVEABLE

}