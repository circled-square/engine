#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include "basic_scene.hpp"
#include "node.hpp"
#include "../rc.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace engine {
    class scene : public engine::basic_scene {
        node m_root;

        engine::renderer m_renderer;
        //for post processing
        rc<const gal::vertex_array> m_whole_screen_vao;
    public:
        virtual ~scene() = default;
        scene();
        scene(scene&& o);

        virtual void render(float frame_time) final;
        virtual void render_ui(float frame_time) {}

        virtual void reheat() final;
        const char* get_name() const override { return "unnamed scene"; }

        node& get_root();
        node& get_node(std::string_view path);
    };
}

#endif // ENGINE_SCENE_HPP
