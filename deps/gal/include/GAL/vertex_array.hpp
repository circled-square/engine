#ifndef GAL_VERTEX_ARRAY_HPP
#define GAL_VERTEX_ARRAY_HPP

#include <tuple>
#include <vector>

#include <GAL/types.hpp>
#include <GAL/api_macro.hpp>
#include "buffer/vertex_buffer.hpp"
#include "buffer/index_buffer.hpp"


namespace gal {

    struct vertex_layout {
        struct vertex_array_attrib { 
            // note: reordering fields in this struct changes its interface, since it is not constructed with explicit constructors
            uint index, offset, type_id;
            sint size;
            uint vao_vbo_bind_index; 
            bool normalized;
        };

        size_t vertex_size = 0; //only optionally set
        std::vector<vertex_array_attrib> attribs;
    };

    class vertex_array {
        uint m_vao;
        std::vector<vertex_buffer> m_vbos;
        std::vector<index_buffer> m_ibos;
    public:

        GAL_API vertex_array(vertex_array&& o);
        GAL_API vertex_array(vertex_buffer vbo, index_buffer ibo, vertex_layout layout = vertex_layout());
        GAL_API vertex_array(std::vector<vertex_buffer> vbos, std::vector<index_buffer> ibos, vertex_layout layout = vertex_layout());

        template<typename vertex_t>
        static vertex_array make(vertex_buffer vbo, index_buffer ibo) {
            static_assert(sizeof(vertex_t) == vertex_t::layout_t::vertex_size(), "the vertex_t type passed to vertex_array::make has a size different than the one advertised by vertex_t::layout_t"); // validate the vertex layout
            return vertex_array(std::move(vbo), std::move(ibo), vertex_t::layout_t::to_vertex_layout());
        }

        GAL_API ~vertex_array();

        GAL_API void specify_attrib(uint attrib_index, uint offset, uint type, sint size, uint vao_vbo_bind_index, bool normalized);
        GAL_API void specify_attribs(const vertex_layout& layout);

        GAL_API void bind(uint ibo_index = 0) const; // bind the vao (and the specified ibo)

        GAL_API size_t get_triangle_count(uint ibo_index = 0) const;
        GAL_API uint get_ibo_element_typeid(uint ibo_index) const;
        GAL_API size_t get_ibo_count() const;
    };



    template <typename... Ts>
    struct static_vertex_layout {
        using tuple_t = std::tuple<Ts...>;

        constexpr static size_t vertex_size() { return sizeof(tuple_t); }

        constexpr static_vertex_layout(Ts...) {} // allows syntax like 'using vertex_t = decltype(static_vertex_layout(pos, normal, tex_coord));'

    private:
        template<typename T>
        static void to_vertex_layout_helper(vertex_layout& layout, uint& index, uint& offset) {
            using vec = scalar_to_vector<T>;
            constexpr uint type_id = gl_type_id<typename vec::value_type>;
            constexpr sint size = vec::length();
            constexpr uint vao_vbo_bind_index = 0;

            layout.attribs.push_back(vertex_layout::vertex_array_attrib { 
                .index=index, .offset=offset, .type_id=type_id, .size=size, .vao_vbo_bind_index=vao_vbo_bind_index
            });

            index++;
            offset += sizeof(T);
        }

    public:
        static vertex_layout to_vertex_layout() {
            vertex_layout layout;
            layout.attribs.reserve(sizeof...(Ts));

            uint index = 0, offset = 0;
            (to_vertex_layout_helper<Ts>(layout, index, offset), ...);
            [[maybe_unused]]
            size_t vertex_layout_vertex_size = offset; //the size of the vertex is equal to the offset where an additional past-the-last attrib would be located
            assert(vertex_layout_vertex_size == vertex_size() && "logic error at static_vertex_layout::to_vertex_layout()");

            layout.vertex_size = vertex_size();

            return layout;
        };
    };
}

#endif //GAL_VERTEX_ARRAY_HPP
