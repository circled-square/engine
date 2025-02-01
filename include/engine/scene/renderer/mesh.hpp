#ifndef ENGINE_SCENE_RENDERER_MESH_HPP
#define ENGINE_SCENE_RENDERER_MESH_HPP

#include <vector>
#include <engine/resources_manager/rc.hpp>
#include "mesh/material.hpp"

namespace engine {
    struct primitive {
        material primitive_material;
        rc<const gal::vertex_array> vao;
    };

    class mesh {
        std::vector<primitive> m_primitives;

    public:
        mesh(material m, rc<const gal::vertex_array> vao);
        mesh(std::vector<primitive> primitives);

        const std::vector<primitive> primitives() const;
    };
}


#endif // ENGINE_SCENE_RENDERER_MESH_HPP
