#ifndef ENGINE_SCENE_RENDERER_HPP
#define ENGINE_SCENE_RENDERER_HPP

#include <GAL/renderer/renderer.hpp>
#include <engine/resources_manager/rc.hpp>
#include "renderer/mesh.hpp"

namespace engine {
    class renderer {
        gal::renderer m_low_level_renderer;
    public:
        void clear(glm::vec4 c = {0,0,0,1});
        void draw(const mesh& mesh, glm::ivec2 output_resolution, glm::mat4 mvp, float frame_time = 0.f);
        gal::renderer& get_low_level_renderer();
    };
}

#endif // ENGINE_SCENE_RENDERER_HPP
