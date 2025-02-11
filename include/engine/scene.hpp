#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include <bit>

#include "scene/node.hpp"
#include "scene/broad_phase_collision.hpp"
#include "scene/application_channel.hpp"

#include <engine/resources_manager/rc.hpp>

namespace engine {
    enum class depth_test_t { disabled = 0, keep_less, keep_more };
    enum class face_culling_t { disabled = 0, front, back };

    struct render_flags_t {
        depth_test_t depth_test = depth_test_t::keep_less;
        bool perform_alpha_blend = true;
        face_culling_t face_culling = face_culling_t::back;
    };

    class scene {
        noderef m_root;
        std::string m_name;
        engine::renderer m_renderer;
        rc<const gal::vertex_array> m_whole_screen_vao; // for post-processing
        render_flags_t m_render_flags;

        application_channel_t m_application_channel;

        pass_all_broad_phase_collision_detector m_bp_collision_detector;
    public:
        scene() = delete;
        scene(std::string s, noderef root, render_flags_t flags = {}, application_channel_t::to_app_t to_app_chan = {});
        scene(scene&& o);

        // prepare() must be called when the scene is inited and when the application switches from a different scene
        // (requires OpenGL to be inited)
        void prepare();

        //update() & render() must be called every frame
        void update();
        void render();

        render_flags_t get_render_flags() { return m_render_flags; }
        void set_render_flags(render_flags_t flags) { m_render_flags = flags; }

        const std::string& get_name() const { return m_name; }

        noderef get_root();
        noderef get_node(std::string_view path);

        //used by engine::application to communicate with the scene
        const application_channel_t::to_app_t& channel_to_app() const;
        rc<scene> get_and_reset_scene_to_change_to();
        application_channel_t::from_app_t& channel_from_app();
    };
}

#endif // ENGINE_SCENE_HPP
