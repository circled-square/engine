#include "engine/scene/renderer.hpp"
#include <engine/resources_manager.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager/rc.hpp>

#define ENGINE_DO_EXPORT
#include <engine/scene.hpp>

namespace engine {
    using glm::mat4;

    template<typename T, typename underlying_type>
    concept MaybeConst = std::same_as<std::remove_const_t<T>, std::remove_const_t<underlying_type>>;

    //pre-order dfs, without recursion
    template<MaybeConst<node> node_t, Callable<void(node_t&)> callable_t>
    inline void depth_first_traversal(node_t& root, const callable_t& callable) {
        std::vector<node_t*> stack;
        stack.push_back(&root);
        while(!stack.empty()) {
            node_t* n = stack.back();
            stack.pop_back();
            //iterate in reverse, so the first child is added last, which means it is visited first
            auto children = n->children();
            for(std::int64_t i = children.size()-1; i >= 0; i--)
                stack.push_back(&children[i]);

            callable(*n);
        }
    }

    //pre+post-order dfs, without recursion
    template<MaybeConst<node> node_t, typename traversal_payload_t, Callable<traversal_payload_t(node_t&, const traversal_payload_t&)> preorder_t, Callable<void(node_t&, const traversal_payload_t&)> postorder_t>
    inline void depth_first_traversal(node_t& root, const traversal_payload_t& root_params, const preorder_t& preorder, const postorder_t& postorder)
    {
        struct stack_entry_t {
            node_t* n;
            // payload to be passed to children when they are visited, both pre- and post-order
            traversal_payload_t p;
            // index of next child to be visited
            size_t i;
        };
        std::vector<stack_entry_t> stack;

        traversal_payload_t root_payload = preorder(root, root_params); // must be done before root is moved out of

        stack.push_back({ &root, std::move(root_payload), 0 });

        while(!stack.empty()) {
            auto& [n, p, i] = stack.back();
            auto n_children = n->children();

            if(i < n_children.size()) {
                node_t& c = n_children[i];
                traversal_payload_t c_payload = preorder(c, p); // must be done before c is moved out of
                i++; //do NOT update i after having pushed, as pushing can possibly invalidate the reference
                stack.push_back({ &c, std::move(c_payload), 0 });
            } else {
                node_t* m = n; // copy value before destroying the entry
                stack.pop_back();

                if(stack.empty()) {
                    postorder(*m, root_params);
                } else {
                    stack_entry_t& father_entry = stack.back();
                    postorder(*m, father_entry.p);
                }
            }
        }
    }

    static void render_tree(renderer& r, const gal::vertex_array& whole_screen_vao, const node& root, const mvp_matrices& viewproj, glm::ivec2 out_res, float frame_time) {
        struct payload_t {
            glm::ivec2 out_res;
            mvp_matrices viewproj;
            const node* vp_node;
        };

        constexpr std::nullptr_t default_framebuffer = nullptr;

        depth_first_traversal(root, payload_t {out_res, viewproj, default_framebuffer},
            //preorder: construction of payload
            [frame_time, &r](const node& n, const payload_t& father_payload) {
                payload_t children_payload;

                // EXPECTS(n.operator rc<const node>());

                // if n is a viewport first setup rendering of children,
                // otherwise render them to the same viewport as n
                if (n.has<viewport>()) {
                    n.get<viewport>().output_resolution_changed(father_payload.out_res);

                    children_payload.vp_node = &n;
                    children_payload.out_res = n.get<viewport>().fbo().resolution();

                    n.get<viewport>().bind_draw();

                    mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)children_payload.out_res.x / children_payload.out_res.y, .1f, 1000.f); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
                    mat4 view_mat = n.get<viewport>().get_active_camera().value_or(mat4(1)).get_view_mat();
                    children_payload.viewproj = mvp_matrices { .m=glm::mat4(1.), .v=view_mat, .p=proj_mat };
                    r.clear();
                } else {
                    children_payload = father_payload;
                }

                return children_payload;
            },
            //postorder: rendering and viewport switching
            [frame_time, &r, &whole_screen_vao](const node& n, const payload_t& father_payload){
                // render self
                if (n.has<mesh>()){
                    r.get_low_level_renderer().change_viewport_size(father_payload.out_res);

                    mvp_matrices mvp = father_payload.viewproj;
                    mvp.m = n.get_global_transform();
                    r.draw(n.get<mesh>(), father_payload.out_res,  mvp, frame_time);
                }

                // only rebind the viewport if n is a viewport (to unbind n and bind whatever its ancestor viewport is)
                if (n.has<viewport>()) {
                    //bind the correct output fbo
                    if(father_payload.vp_node) {
                        EXPECTS(father_payload.vp_node->has<viewport>());
                        father_payload.vp_node->get<viewport>().bind_draw();
                    } else
                        framebuffer::unbind();

                    r.get_low_level_renderer().change_viewport_size(father_payload.out_res);
                }
            });
    }

    //sets the cameras for all viewports in the hierarchy, and returns the camera to use for the default framebuffer.
    // TODO: eliminate recursion from this function: profiling shows it is more relevant than I thought
    // TODO: check whether we can reduce the number of depth_first_traversals per frame
    [[nodiscard]]
    static std::optional<camera> set_cameras_recursive_bk(node& n, node* forefather_vp_node = nullptr) {
        std::optional<camera> default_fb_camera = std::nullopt;

        //if this is a camera set it as active for it forefather (/default) viewport
        if(n.has<camera>()) {
            n.get<camera>().set_view_mat(glm::inverse(n.get_global_transform()));

            if(forefather_vp_node) {
                EXPECTS(forefather_vp_node->has<viewport>());
                forefather_vp_node->get<viewport>().set_active_camera(n.get<camera>());
            } else {
                default_fb_camera = n.get<camera>();
            }
        }

        //if this is a viewport set its camera to null and use it for its children
        if(n.has<viewport>())
            n.get<viewport>().set_active_camera(std::nullopt);
        node* children_vp = n.has<viewport>() ? &n : forefather_vp_node;

        // process children
        for(node& child : n.children()) {
            auto other_camera = set_cameras_recursive_bk(child, children_vp);

            if(!default_fb_camera)
                default_fb_camera = other_camera;
        }
        return default_fb_camera;
    }
    [[nodiscard]]
    static std::optional<camera> set_cameras_no_recursion(node& root) {
        std::optional<camera> default_fb_camera = std::nullopt;

        struct payload_t {
            node* ancestor_vp = nullptr;
        };

        depth_first_traversal(root, payload_t{},
            // [&](const rc<node>& n, const payload_t& father_pl) { // preorder
            [&](node& n, const payload_t& father_pl) { // preorder
                payload_t children_pl = father_pl;

                //if this is a camera set it as active for it forefather (/default) viewport
                if(n.has<camera>()) {
                    n.get<camera>().set_view_mat(glm::inverse(n.get_global_transform()));

                    if(father_pl.ancestor_vp) {
                        EXPECTS(father_pl.ancestor_vp->has<viewport>());
                        father_pl.ancestor_vp->get<viewport>().set_active_camera(n.get<camera>());
                    } else {
                        default_fb_camera = n.get<camera>();
                    }
                }

                //if this is a viewport set its camera to null and use it for its children
                if(n.has<viewport>()) {
                    n.get<viewport>().set_active_camera(std::nullopt);
                    children_pl.ancestor_vp = &n;
                }

                return children_pl;
            },
            [](node&, const payload_t&) {} // postorder
        );

        return default_fb_camera;
    }


    scene::scene(std::string name, std::unique_ptr<node> root, application_channel_t::to_app_t to_app_chan)
        : m_root(std::move(root)),
          m_name(std::move(name)),
          m_renderer(),
          m_whole_screen_vao(get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)),
          m_render_flags(),
          m_application_channel(std::move(to_app_chan), application_channel_t::from_app_t{ .scene_name = m_name }) {
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

        std::optional<camera> default_fb_camera = set_cameras_no_recursion(get_root());
        // std::optional<camera> default_fb_camera = set_cameras_recursive_bk(get_root(), std::nullopt);
        // std::optional<camera> default_fb_camera = std::nullopt;

        m_renderer.clear(m_application_channel.to_app().clear_color);
        mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)resolution.x / resolution.y, .1f, 1000.f); // TODO: fovy and znear/zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
        mat4 view_mat = default_fb_camera ? default_fb_camera->get_view_mat() : mat4(1);
        mvp_matrices viewproj { .m=mat4(1.), .v=view_mat, .p=proj_mat };
        render_tree(m_renderer, *m_whole_screen_vao, get_root(), viewproj, resolution, frame_time);
    }

    void scene::update() {
        // process nodes
        depth_first_traversal(get_root(), [&](node& n){
            visit_optional(n.get_script(), [&](auto& s) {
                s.process(n, m_application_channel);
            });
        });

        // TODO: currently resubscribing all colliders at every update: is it ok? ideally colliders would subscribe/unsubscribe themselves, making this unnecessary
        m_bp_collision_detector.reset_subscriptions();

        depth_first_traversal(get_root(), [&](node& n){
            if(n.has<collision_shape>())
                m_bp_collision_detector.subscribe(&n);
        });

        m_bp_collision_detector.check_collisions_and_trigger_reactions();
    }

    void scene::prepare() {
        m_renderer.get_low_level_renderer().set_render_flags(m_render_flags);
    }

    node& scene::get_root() { return *m_root; }

    node& scene::get_node(std::string_view path) {
        if(path[0] != '/')
            throw invalid_path_exception(path);

        std::string_view subpath(path.begin()+1, path.end());
        return m_root->get_descendant_from_path(subpath);
    }


} // namespace engine
