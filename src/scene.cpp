#include "glm/gtc/matrix_inverse.hpp"
#include <engine/scene.hpp>
#include <engine/resources_manager.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {
    static void render_node(renderer& r, const gal::vertex_array& whole_screen_vao, const node& n, glm::mat4 parent_transform, glm::mat4 viewproj_mat,
                            glm::ivec2 output_resolution, float frame_time, const viewport* output_vp) {
        glm::mat4 transform = parent_transform * n.transform();

        glm::ivec2 children_out_res;
        glm::mat4 children_viewproj;
        const viewport* children_vp;

        // if n is a viewport first setup rendering of children
        if (n.has<viewport>()) {
            n.get<viewport>().output_resolution_changed(output_resolution);

            children_vp = &n.get<viewport>();
            children_out_res = children_vp->fbo().resolution();

            children_vp->bind_draw();

            glm::mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)children_out_res.x / children_out_res.y, .1f, 1000.f); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
            glm::mat4 view_mat = children_vp->get_active_camera() ?  children_vp->get_active_camera()->get_view_mat() : glm::mat4(1);
            children_viewproj = proj_mat * view_mat;
            r.clear();
        } else {
            children_out_res = output_resolution;
            children_vp = output_vp;
            children_viewproj = viewproj_mat;
        }

        glViewport(0,0, children_out_res.x,  children_out_res.y);


        // render self
        if (n.has<mesh>())
            r.draw(n.get<mesh>(), children_out_res, viewproj_mat * transform, frame_time);

        // render children
        for(const auto& child : n.children())
            render_node(r, whole_screen_vao, child, transform, children_viewproj, children_out_res, frame_time, children_vp);


        // if n is a viewport render it to the output_vp
        if (n.has<viewport>()) {
            //bind the correct output fbo
            if(output_vp)
                output_vp->bind_draw();
            else
                framebuffer::unbind();

            glViewport(0,0, output_resolution.x,  output_resolution.y);

            //post processing
            n.get<viewport>().postfx_material().bind_and_set_uniforms(glm::mat4(), output_resolution, frame_time);

            // see red background? bad thing
            r.get_low_level_renderer().clear(glm::vec4(1,0,0,1));
            // draw
            r.get_low_level_renderer().draw(whole_screen_vao, n.get<viewport>().postfx_material().get_shader()->get_program());
        }
    }

    static void process_node(node& n, glm::mat4 parent_transform, application_channel_t& app_chan) {
        glm::mat4 transform = parent_transform * n.transform();

        n.process(app_chan);

        // process children
        for(auto& child : n.children()) {
            process_node(child, transform, app_chan);
        }
    }

    [[nodiscard]]
    static const camera* set_cameras(node& n, glm::mat4 parent_transform, viewport* forefather_vp) {
        glm::mat4 transform = parent_transform * n.transform();
        const camera* default_fb_camera = nullptr;

        //if this is a camera set it as active for it forefather (/default) viewport
        if(n.has<camera>()) {
            n.get<camera>().set_view_mat(glm::inverse(transform));

            if(forefather_vp)
                forefather_vp->set_active_camera(&n.get<camera>());
            else
                default_fb_camera = &n.get<camera>();
        }

        //if this is a viewport set its camera to null and use it for its children
        if(n.has<viewport>())
            n.get<viewport>().set_active_camera(nullptr);
        viewport* children_vp = n.has<viewport>() ? &n.get<viewport>() : forefather_vp;

        // process children
        for(auto& child : n.children()) {
            auto other_camera = set_cameras(child, transform, children_vp);

            if(!default_fb_camera)
                default_fb_camera = other_camera;
        }
        return default_fb_camera;
    }

    scene::scene(std::string name, node root, render_flags flags, application_channel_t::to_app_t to_app_chan)
        : m_root(std::move(root)),
          m_name(std::move(name)),
          m_renderer(),
          m_whole_screen_vao(get_rm().get_whole_screen_vao()),
          m_render_flags(std::move(flags)),
          m_application_channel(std::move(to_app_chan), application_channel_t::from_app_t()) {
        //the root of a scene's name should always be unnamed.
        EXPECTS(m_root.name().empty());
    }

    scene::scene(scene &&o)
        :  m_root(std::move(o.m_root)),
          m_name(std::move(o.m_name)),
          m_renderer(std::move(o.m_renderer)),
          m_whole_screen_vao(std::move(o.m_whole_screen_vao)),
          m_render_flags(std::move(o.m_render_flags)),
          m_application_channel(std::move(o.m_application_channel)) {}


    void scene::render() {
        glm::ivec2 resolution = m_application_channel.from_app.framebuffer_size;
        float frame_time = m_application_channel.from_app.frame_time;

        const camera* default_fb_camera = set_cameras(get_root(), glm::mat4(1), nullptr);

        m_renderer.clear();
        glm::mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)resolution.x / resolution.y, .1f, 1000.f); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
        glm::mat4 view_mat = default_fb_camera ? default_fb_camera->get_view_mat() : glm::mat4(1);
        glm::mat4 viewproj_mat = proj_mat * view_mat;
        render_node(m_renderer, *m_whole_screen_vao, get_root(), glm::mat4(1), viewproj_mat, resolution, frame_time, nullptr);
    }

    void scene::update() {
        process_node(get_root(), glm::mat4(1), m_application_channel);

        // TODO: currently resubscribing all colliders at every update: is it ok?
        m_bp_collision_detector.reset_subscriptions();

        std::vector<node*> dfs_stack;
        dfs_stack.push_back(&get_root());
        while(!dfs_stack.empty()) {
            node* n = dfs_stack.back();
            dfs_stack.pop_back();
            for(node& c : n->children())
                dfs_stack.push_back(&c);

            if(n->has<collision_shape>())
                m_bp_collision_detector.subscribe(*n);
        }

        m_bp_collision_detector.check_collisions_and_trigger_reactions();
    }

    void scene::prepare() {
        if(m_render_flags.perform_alpha_blend) {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glEnable(GL_BLEND);
        }

        if(m_render_flags.depth_test == depth_test_t::keep_less) {
            glEnable(GL_DEPTH_TEST); // Enable depth test
            glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
        } else if(m_render_flags.depth_test == depth_test_t::keep_more) {
            glEnable(GL_DEPTH_TEST); // Enable depth test
            glDepthFunc(GL_LESS); // Accept fragment if it is father from the camera than the former one
        }

        if(m_render_flags.face_culling == face_culling_t::back) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else if(m_render_flags.face_culling == face_culling_t::front) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }
    }

    const application_channel_t::to_app_t &scene::channel_to_app() const {
        return m_application_channel.to_app;
    }

    rc<scene> scene::get_and_reset_scene_to_change_to() { return std::move(m_application_channel.to_app.scene_to_change_to); }

    application_channel_t::from_app_t &scene::channel_from_app() {
        return m_application_channel.from_app;
    }

    node& scene::get_root() { return m_root; }

    node& scene::get_node(std::string_view path) {
        if(path[0] != '/')
            throw std::runtime_error(std::format("the first character of a path passed to scene::get_from_path must be '/'; instead path = \"{}\"", path));

        std::string_view subpath(path.begin()+1, path.end());
        return m_root.get_from_path(subpath);
    }

    application_channel_t::application_channel_t(to_app_t to_app, from_app_t from_app) : to_app(std::move(to_app)), from_app(std::move(from_app)) {}

} // namespace engine
