#ifndef ENGINE_SCENE_RENDERER_MESH_HPP
#define ENGINE_SCENE_RENDERER_MESH_HPP

#include <vector>
#include <engine/resources_manager/rc.hpp>
#include "mesh/material.hpp"
#include <engine/utils/api_macro.hpp>

namespace engine {
    struct primitive {
        material primitive_material;
        rc<const gal::vertex_array> vao;
    };

    class mesh {
        std::vector<primitive> m_primitives;

    public:
        ENGINE_API mesh(material m, rc<const gal::vertex_array> vao);
        ENGINE_API mesh(std::vector<primitive> primitives);

        ENGINE_API const std::vector<primitive>& primitives() const;
        ENGINE_API std::vector<primitive>& primitives();
    };
}


#endif // ENGINE_SCENE_RENDERER_MESH_HPP
