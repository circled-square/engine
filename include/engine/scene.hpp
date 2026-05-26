#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include "scene/node.hpp"
#include "scene/broad_phase_collision.hpp"
#include "scene/application_channel.hpp"

#include <engine/resources_manager/rc.hpp>
#include <engine/utils/api_macro.hpp>

namespace engine {
    class scene {
        node m_root;

        std::string m_name;
        engine::renderer m_renderer;
        gal::render_flags m_render_flags;
        rc<const gal::vertex_array> m_whole_screen_vao; // for post-processing

        application_channel_t m_application_channel;

        pass_all_broad_phase_collision_detector m_bp_collision_detector;
    public:
        scene() = delete;
        //TODO: these should not be ENGINE_API
        ENGINE_API scene(std::string s, node root, application_channel_t::to_app_t to_app_chan = {});
        scene(scene&& o) : m_root(o.m_root), m_name(std::move(o.m_name)), m_render_flags(std::move(o.m_render_flags)),
            m_whole_screen_vao(std::move(o.m_whole_screen_vao)),
            m_application_channel(std::move(o.m_application_channel)), m_bp_collision_detector(std::move(o.m_bp_collision_detector))
        {
            o.m_root = node(); // avoid o.~scene() deallocating the scene tree
        }
        ~scene();

        // prepare() is called when the scene is inited and when the application switches from a different scene
        // (requires OpenGL to be inited)
        void prepare();

        //update() & render() are called every frame
        void update();
        void render();

        gal::render_flags get_render_flags() { return m_render_flags; }
        void set_render_flags(gal::render_flags flags) { m_render_flags = flags; }

        const std::string& get_name() const { return m_name; }

        node get_root();
        node get_node(std::string_view path);
        [[nodiscard]] node into_node_tree() {
            node ret = std::move(m_root);
            m_root = node();
            return ret;
        }

        //used by engine::application to communicate with the scene
        const application_channel_t& app_channel() const { return m_application_channel; }
        application_channel_t& app_channel() { return m_application_channel; }
    };

    class invalid_path_exception : public std::exception {
        std::string m_what;
    public:
        invalid_path_exception(std::string_view path) : m_what(std::format("the first character of a path passed to scene::get_node must be '/'; instead path = \"{}\"", path)) {}
        const char* what() const noexcept override { return m_what.c_str(); }
    };
}

#endif // ENGINE_SCENE_HPP
