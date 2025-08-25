#include <engine/scene.hpp>
#include <engine/resources_manager.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {
    using glm::mat4;

    //pre-order dfs, without explicit recursion
    template<typename node_t>
    inline void depth_first_traversal(node_t root, const auto& callable)
        requires(std::same_as<node_t, node> || std::same_as<node_t, const_node>)
    {
        std::vector<node_t> stack;
        stack.push_back(root);
        while(!stack.empty()) {
            node n = stack.back();
            stack.pop_back();
            for(node c : n->children())
                stack.push_back(std::move(c));

            callable(std::move(n));
        }
    }

    constexpr std::nullptr_t default_framebuffer = nullptr;

    static void render_node(renderer& r, const gal::vertex_array& whole_screen_vao, rc<const node_data> n, const mat4& viewproj_mat,
                            glm::ivec2 output_resolution, float frame_time, rc<const node_data> output_vp_node) {
        glm::ivec2 children_out_res;
        mat4 children_viewproj;
        rc<const node_data> children_vp_node;

        // if n is a viewport first setup rendering of children,
        // otherwise render them to the same viewport as n
        if (n->has<viewport>()) {
            n->get<viewport>().output_resolution_changed(output_resolution);

            children_vp_node = n;
            children_out_res = children_vp_node->get<viewport>().fbo().resolution();

            children_vp_node->get<viewport>().bind_draw();

            mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)children_out_res.x / children_out_res.y, .1f, 1000.f); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
            mat4 view_mat = children_vp_node->get<viewport>().get_active_camera().value_or(mat4(1)).get_view_mat();
            children_viewproj = proj_mat * view_mat;
            r.clear();
        } else {
            children_out_res = output_resolution;
            children_vp_node = output_vp_node;
            children_viewproj = viewproj_mat;
        }

        glViewport(0,0, children_out_res.x,  children_out_res.y);


        // render self
        if (n->has<mesh>())
            r.draw(n->get<mesh>(), children_out_res, viewproj_mat * n->get_global_transform(), frame_time);

        // render children
        for(rc<const node_data> child : n->children())
            render_node(r, whole_screen_vao, child, children_viewproj, children_out_res, frame_time, children_vp_node);

        /* TODO: viewports are always automatically rendered, without any transformation, to the first viewport in their ancestors.
         * This should probably configurable, for example it should be possible to use them as textures for another material or something like that
         */
        // if n is a viewport render it to the output_vp
        if (n->has<viewport>()) {
            //bind the correct output fbo
            if(output_vp_node) {
                EXPECTS(output_vp_node->has<viewport>());
                output_vp_node->get<viewport>().bind_draw();
            } else
                framebuffer::unbind();

            glViewport(0,0, output_resolution.x,  output_resolution.y);

            //post processing
            n->get<viewport>().postfx_material().bind_and_set_uniforms(mat4(), output_resolution, frame_time);

            // see red background? bad thing
            r.get_low_level_renderer().clear(glm::vec4(1,0,0,1));
            // draw
            r.get_low_level_renderer().draw(whole_screen_vao, n->get<viewport>().postfx_material().get_shader()->get_program());
        }
    }

    //sets the cameras for all viewports in the hierarchy, and returns the camera to use for the default framebuffer.
    [[nodiscard]]
    static std::optional<camera> set_cameras(node n, rc<node_data> forefather_vp_node) {
        std::optional<camera> default_fb_camera = std::nullopt;

        //if this is a camera set it as active for it forefather (/default) viewport
        if(n->has<camera>()) {
            n->get<camera>().set_view_mat(glm::inverse(n->get_global_transform()));

            if(forefather_vp_node) {
                EXPECTS(forefather_vp_node->has<viewport>());
                forefather_vp_node->get<viewport>().set_active_camera(n->get<camera>());
            } else {
                default_fb_camera = n->get<camera>();
            }
        }

        //if this is a viewport set its camera to null and use it for its children
        if(n->has<viewport>())
            n->get<viewport>().set_active_camera(std::nullopt);
        rc<node_data> children_vp = n->has<viewport>() ? n : std::move(forefather_vp_node);

        // process children
        for(node child : n->children()) {
            auto other_camera = set_cameras(child, children_vp);

            if(!default_fb_camera)
                default_fb_camera = other_camera;
        }
        return default_fb_camera;
    }

    scene::scene(std::string name, node root, render_flags_t flags, application_channel_t::to_app_t to_app_chan)
        : m_root(std::move(root)),
          m_name(std::move(name)),
          m_renderer(),
          m_whole_screen_vao(get_rm().get_whole_screen_vao()),
          m_render_flags(std::move(flags)),
          m_application_channel(std::move(to_app_chan), application_channel_t::from_app_t()) {
        //the root of a scene's name should always be unnamed.
        EXPECTS(m_root->name().empty());
    }

    scene::scene(scene &&o)
        :  m_root(std::move(o.m_root)),
          m_name(std::move(o.m_name)),
          m_renderer(std::move(o.m_renderer)),
          m_whole_screen_vao(std::move(o.m_whole_screen_vao)),
          m_render_flags(std::move(o.m_render_flags)),
          m_application_channel(std::move(o.m_application_channel)) {}


    void scene::render() {
        glm::ivec2 resolution = m_application_channel.from_app().framebuffer_size;
        float frame_time = m_application_channel.from_app().frame_time;

        std::optional<camera> default_fb_camera = set_cameras(get_root(), nullptr);

        m_renderer.clear();
        mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)resolution.x / resolution.y, .1f, 1000.f); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
        mat4 view_mat = default_fb_camera ? default_fb_camera->get_view_mat() : mat4(1);
        mat4 viewproj_mat = proj_mat * view_mat;
        render_node(m_renderer, *m_whole_screen_vao, get_root(), viewproj_mat, resolution, frame_time, nullptr);
    }

    void scene::update() {
        // process nodes
        depth_first_traversal(get_root(), [&](node n){
            visit_optional(n->get_script(), [&](auto& s) {
                s.process(n, m_application_channel);
            });
        });

        // TODO: currently resubscribing all colliders at every update: is it ok? ideally colliders would subscribe/unsubscribe themselves, making this unnecessary
        m_bp_collision_detector.reset_subscriptions();

        depth_first_traversal(get_root(), [&](node n){
            if(n->has<collision_shape>())
                m_bp_collision_detector.subscribe(*n);
        });

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

    node scene::get_root() { return m_root; }

    node scene::get_node(std::string_view path) {
        if(path[0] != '/')
            throw invalid_path_exception(path);

        std::string_view subpath(path.begin()+1, path.end());
        return m_root->get_from_path(subpath);
    }


} // namespace engine
