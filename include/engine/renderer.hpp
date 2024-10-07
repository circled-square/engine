#ifndef ENGINE_RENDERER_HPP
#define ENGINE_RENDERER_HPP

#include <GAL/renderer/renderer.hpp>
#include "rc.hpp"
#include "material.hpp"

namespace engine {
    class camera {
        glm::mat4 m_view_mat;
    public:
        camera(glm::mat4 view_matrix = glm::mat4(1));
        glm::mat4 get_view_mat() const;
        void set_view_mat(glm::mat4 view_mat);

    };

    struct primitive {
        material primitive_material;
        rc<const gal::vertex_array> vao;
    };

    class mesh {
        std::vector<primitive> m_primitives;

    public:
        mesh(material m, rc<const gal::vertex_array> vao) {
            m_primitives.emplace_back(std::move(m), std::move(vao));
        }
        mesh(std::vector<primitive> primitives) : m_primitives(std::move(primitives)) {}
        const std::vector<primitive> primitives() const { return m_primitives; }
    };

    class renderer {
        gal::renderer m_low_level_renderer;
    public:
        void clear(glm::vec4 c = {0,0,0,1});
        void draw(const mesh& mesh, glm::ivec2 output_resolution, glm::mat4 mvp, float frame_time = 0.f);
        gal::renderer& get_low_level_renderer();
    };
}

#endif // ENGINE_RENDERER_HPP
