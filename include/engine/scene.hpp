#ifndef ENGINE_SCENE_HPP
#define ENGINE_SCENE_HPP

#include "scene/node.hpp"
#include "scene/broad_phase_collision.hpp"
#include "scene/application_channel.hpp"

#include <engine/resources_manager/rc.hpp>
#include <engine/utils/api_macro.hpp>

namespace engine {
    class scene {
        rc<node> m_root;
        std::string m_name;
        engine::renderer m_renderer;
        rc<const gal::vertex_array> m_whole_screen_vao; // for post-processing
        gal::render_flags m_render_flags;

        application_channel_t m_application_channel;

        pass_all_broad_phase_collision_detector m_bp_collision_detector;
    public:
        scene() = delete;
        //TODO: these should not be ENGINE_API
        ENGINE_API scene(std::string s, rc<node> root, application_channel_t::to_app_t to_app_chan = {});
        ENGINE_API scene(scene&& o);

        // prepare() is called when the scene is inited and when the application switches from a different scene
        // (requires OpenGL to be inited)
        void prepare();

        //update() & render() are called every frame
        void update();
        void render();

        gal::render_flags get_render_flags() { return m_render_flags; }
        void set_render_flags(gal::render_flags flags) { m_render_flags = flags; }

        const std::string& get_name() const { return m_name; }

        const rc<node>& get_root();
        rc<node> get_node(std::string_view path);

        //used by engine::application to communicate with the scene
        const application_channel_t& app_channel() const { return m_application_channel; }
        application_channel_t& app_channel() { return m_application_channel; }
    };

    class invalid_path_exception : public std::exception {
        std::string m_what;
    public:
        invalid_path_exception(std::string_view path) : m_what(std::format("the first character of a path passed to scene::get_node must be '/'; instead path = \"{}\"", path)) {}
        virtual const char* what() const noexcept { return m_what.c_str(); }
    };
}

#endif // ENGINE_SCENE_HPP
