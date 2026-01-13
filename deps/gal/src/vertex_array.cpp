#include <GAL/vertex_array.hpp>
#include <glad/glad.h>
#include <slogga/log.hpp>

namespace gal {
    vertex_array::vertex_array(std::vector<vertex_buffer> vbos, std::vector<index_buffer> ibos, vertex_layout layout) : m_vbos(std::move(vbos)), m_ibos(std::move(ibos)) {
        glCreateVertexArrays(1, &m_vao);

        for(size_t i = 0; i < m_vbos.size(); i++) {
            const uint vao_vbo_bind_index = i;
            // bind {3} as the {2}th vbo to the vao {1}, starting from {4} within the buffer. the stride between vertices is {5}
            glVertexArrayVertexBuffer(m_vao, vao_vbo_bind_index, m_vbos[i].get_gl_id(), 0, (int)m_vbos[i].get_stride());
        }

        this->specify_attribs(layout);
    }

    inline std::vector<vertex_buffer> vbo_to_vbos_vec(vertex_buffer vbo) {
        std::vector<vertex_buffer> vbos;
        vbos.reserve(1);
        vbos.push_back(std::move(vbo));
        return vbos;
    }

    inline std::vector<index_buffer> ibo_to_ibos_vec(index_buffer ibo) {
        std::vector<index_buffer> ibos;
        ibos.reserve(1);
        ibos.push_back(std::move(ibo));
        return ibos;
    }

    vertex_array::vertex_array(vertex_buffer vbo, index_buffer ibo, vertex_layout layout)
        : vertex_array(vbo_to_vbos_vec(std::move(vbo)), ibo_to_ibos_vec(std::move(ibo)), std::move(layout)) {}

    vertex_array::vertex_array(vertex_array&& o) : m_vao(o.m_vao), m_vbos(std::move(o.m_vbos)), m_ibos(std::move(o.m_ibos)) {
        o.m_vao = 0;
    }

    vertex_array::~vertex_array() {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    void vertex_array::specify_attrib(uint attrib_index, uint offset, uint type_id, uint size, uint vao_vbo_bind_index) {
        //the vertex array contains vertices with the following attribs:
        glEnableVertexArrayAttrib(m_vao, attrib_index); //enable the attrib {2} for the vao {1} (can be done after call to glVertexAttribPointer I think)
        //the {1} vao's {2}th attrib is {3} elements of type {4}; they do/don't({5}) need normalization; {6} is the offset of the attrib from the start of the vertex.
        glVertexArrayAttribFormat(m_vao, attrib_index, size, type_id, GL_FALSE, offset); 
        glVertexArrayAttribBinding(m_vao, attrib_index, vao_vbo_bind_index); // bind the attrib {2} to the {3}th vbo of the vao {1}
    }

    void vertex_array::specify_attribs(const vertex_layout& layout) {
        #ifndef NDEBUG
            if(layout.vertex_size != 0) { //layout.vertex_size is only optionally set
                size_t vbos_stride_sum = 0;
                for(auto& vbo : m_vbos) {
                    vbos_stride_sum += vbo.get_stride();
                }
                assert(vbos_stride_sum == layout.vertex_size);
            }
        #endif
        
        for(const vertex_layout::vertex_array_attrib& attrib : layout.attribs) {
            specify_attrib(attrib.index, attrib.offset, attrib.type_id, attrib.size, attrib.vao_vbo_bind_index);
        }
    }

    void vertex_array::bind(uint ibo_index) const {
        // bind the ibo {2} to the vao {1}
        glVertexArrayElementBuffer(m_vao, m_ibos[ibo_index].get_gl_id());

        glBindVertexArray(m_vao);
    }

    size_t vertex_array::get_triangle_count(uint ibo_index) const {
        return m_ibos[ibo_index].get_triangle_count(); 
    }
    
    uint vertex_array::get_ibo_element_typeid(uint ibo_index) const {
        return m_ibos[ibo_index].get_element_typeid();
    }

    size_t vertex_array::get_ibo_count() const {
        return m_ibos.size();
    }
}
