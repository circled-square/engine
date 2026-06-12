#include "engine/scene/renderer.hpp"
#include <engine/resources_manager.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/resources_manager/rc.hpp>
#include <engine/scene.hpp>
#include <engine/utils/dfs.hpp>

namespace engine {
    using glm::mat4;

    constexpr float fovy = glm::pi<float>() / 4, znear = .1f, zfar = 1000.f;

    static void render_tree(renderer& r, const gal::vertex_array& whole_screen_vao, node root, const mvp_matrices& viewproj, glm::ivec2 out_res, float frame_time) {
        // TODO: make a separate stack from the dfs payload stack so we don't keep push/popping viewport info for no reason
        struct payload_t {
            glm::ivec2 out_res;
            mvp_matrices viewproj;
            node vp_node;
        };

        constexpr node default_framebuffer = node();

        depth_first_traversal(root, payload_t {out_res, viewproj, default_framebuffer},
            //preorder: construction of payload
            [frame_time, &r](node n, const payload_t& father_payload) {
                payload_t children_payload{};

                // if n is a viewport first setup rendering of children,
                // otherwise render them to the same viewport as n
                if (optional_ref<viewport> vp = n.get<viewport>()) {
                    vp->output_resolution_changed(father_payload.out_res);

                    children_payload.vp_node = n;
                    children_payload.out_res = vp->fbo().resolution();

                    vp->bind_draw();

                    float aspect_ratio = float(children_payload.out_res.x) / float(children_payload.out_res.y);
                    mat4 proj_mat = glm::perspective(fovy, aspect_ratio, znear, zfar); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
                    mat4 view_mat = vp->get_active_camera().value_or(mat4(1)).get_view_mat();
                    children_payload.viewproj = mvp_matrices { .m=glm::mat4(1.), .v=view_mat, .p=proj_mat };
                    r.clear();
                } else {
                    children_payload = father_payload;
                }

                return children_payload;
            },
            //postorder: rendering and viewport switching
            [frame_time, &r](node n, const payload_t& father_payload){
                // render self
                if (optional_ref<mesh> n_mesh = n.get<mesh>()) {
                    r.get_low_level_renderer().change_viewport_size(father_payload.out_res);

                    mvp_matrices mvp = father_payload.viewproj;
                    mvp.m = n.get_global_transform();
                    r.draw(*n_mesh, father_payload.out_res,  mvp, frame_time);
                }

                // only rebind the viewport if n is a viewport (to unbind n and bind whatever its ancestor viewport is)
                if (optional_ref<viewport> vp = n.get<viewport>()) {
                    //bind the correct output fbo
                    if(father_payload.vp_node.ecs_id() != null_ecs_id) {
                        father_payload.vp_node.get<viewport>()->bind_draw();
                    } else {
                        framebuffer::unbind();
                    }

                    r.get_low_level_renderer().change_viewport_size(father_payload.out_res);
                }
            });
    }

    //sets the cameras for all viewports in the hierarchy, and returns the camera to use for the default framebuffer.
    // TODO: check whether we can reduce the number of depth_first_traversals per frame    [[nodiscard]]
    static std::optional<camera> set_cameras(node root) {
        std::optional<camera> default_fb_camera = std::nullopt;

        struct payload_t {
            node ancestor_vp;
        };

        depth_first_traversal(root, payload_t{},
            // [&](const rc<node>& n, const payload_t& father_pl) { // preorder
            [&](node n, const payload_t& father_pl) { // preorder
                payload_t children_pl = father_pl;

                //if this is a camera set it as active for it forefather (/default) viewport
                if(auto cam = n.get<camera>()) {
                    cam->set_view_mat(glm::inverse(n.get_global_transform()));

                    if(father_pl.ancestor_vp.ecs_id() != null_ecs_id) {
                        father_pl.ancestor_vp.get<viewport>()->set_active_camera(*cam);
                    } else {
                        default_fb_camera = *cam;
                    }
                }

                //if this is a viewport set its camera to null and use it for its children
                if(auto vp = n.get<viewport>()) {
                    vp->set_active_camera(std::nullopt);
                    children_pl.ancestor_vp = n;
                }

                return children_pl;
            },
            [](node, const payload_t&) {} // postorder
        );

        return default_fb_camera;
    }


    scene::scene(std::string name, node root, application_channel_t::to_app_t to_app_chan)
        : m_root(std::move(root)),
          m_name(std::move(name)),
          m_renderer(),
          m_render_flags(),
          m_whole_screen_vao(get_rm().load<gal::vertex_array>(internal_resource_name_t::whole_screen_vao)),
          m_application_channel(std::move(to_app_chan), application_channel_t::from_app_t{ .scene_name = m_name }) {
        EXPECTS(m_root.ecs_id() != null_ecs_id);
        EXPECTS(m_root.name().empty()); //the root node should always be unnamed.
    }

    void scene::render() {
        glm::ivec2 resolution = m_application_channel.from_app().framebuffer_size;
        float frame_time = m_application_channel.from_app().frame_time;

        std::optional<camera> default_fb_camera = set_cameras(get_root());

        m_renderer.clear(m_application_channel.to_app().clear_color);

        float aspect_ratio = float(resolution.x) / float(resolution.y);
        mat4 proj_mat = glm::perspective(fovy, aspect_ratio, znear, zfar); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
        mat4 view_mat = default_fb_camera ? default_fb_camera->get_view_mat() : mat4(1);
        mvp_matrices viewproj { .m=mat4(1.), .v=view_mat, .p=proj_mat };

        render_tree(m_renderer, *m_whole_screen_vao, get_root(), viewproj, resolution, frame_time);
        m_renderer.finalize_frame();

        if (get_root().children().size() == 0) {
            slogga::stdout_log.warn("scene \"{}\"'s root  has 0 children", m_name);
        }
    }

    void scene::update() {
        // process nodes
        depth_first_traversal(get_root(), [&](node n){
            if(auto s = n.get<script>()) {
                s->process(n, m_application_channel);
            }
        });

        // TODO: currently resubscribing all colliders at every update: is it ok? ideally colliders would subscribe/unsubscribe themselves, making this unnecessary
        m_bp_collision_detector.reset_subscriptions();

        depth_first_traversal(get_root(), [&](node n){
            if(n.get<rc<const collision_shape>>())
                m_bp_collision_detector.subscribe(n);
        });

        m_bp_collision_detector.check_collisions_and_trigger_reactions();
    }

    scene::~scene() {
        if(m_root.ecs_id() != null_ecs_id) {
            get_rm().ecs().release_id(m_root.ecs_id());
        }
    }

    void scene::prepare() {
        m_renderer.get_low_level_renderer().set_render_flags(m_render_flags);
    }

    node scene::get_root() { return m_root; }

    node scene::get_node(std::string_view path) {
        if(path.at(0) != '/')
            throw invalid_path_exception(path);

        std::string_view subpath = path.substr(1);
        return m_root.get_descendant_from_path(subpath);
    }


} // namespace engine
