#include "glm/gtc/matrix_inverse.hpp"
#include <engine/scene/scene.hpp>
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

    static void process_node(node& n, glm::mat4 parent_transform) {
        glm::mat4 transform = parent_transform * n.transform();

        // n.set_global_transform(transform);
        // n.process();

        // process children
        for(auto& child : n.children()) {
            process_node(child, transform);
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

    scene::scene()
        : engine::basic_scene(),
        m_renderer(),
        m_root(""),
        m_whole_screen_vao(get_rm().get_whole_screen_vao())
    {}

    scene::scene(scene &&o)
        : engine::basic_scene(std::move(o)), m_renderer(std::move(o.m_renderer)), m_root(std::move(o.m_root)),
          m_whole_screen_vao(std::move(o.m_whole_screen_vao)) {}


    void scene::render(float frame_time) {
        glm::ivec2 resolution = application_channel().from_app.framebuffer_size;

        process_node(get_root(), glm::mat4(1)); // TODO: move this in update() final;

        const camera* default_fb_camera = set_cameras(get_root(), glm::mat4(1), nullptr);

        m_renderer.clear();
        glm::mat4 proj_mat = glm::perspective(glm::pi<float>() / 4, (float)resolution.x / resolution.y, .1f, 1000.f); // TODO: fovy and znear and zfar are opinionated choices, and should be somehow parameterized (probably through the camera/viewport)
        glm::mat4 view_mat = default_fb_camera ? default_fb_camera->get_view_mat() : glm::mat4(1);
        glm::mat4 viewproj_mat = proj_mat * view_mat;
        render_node(m_renderer, *m_whole_screen_vao, get_root(), glm::mat4(1), viewproj_mat, resolution, frame_time, nullptr);

        this->render_ui(frame_time);
    }

    void scene::reheat() {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        glEnable(GL_BLEND);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    node& scene::get_root() { return m_root; }

    node& scene::get_node(std::string_view path) {
        if(path[0] != '/')
            throw std::runtime_error(std::format("the first character of a path passed to scene::get_from_path must be '/'; instead path = \"{}\"", path));

        std::string_view subpath(path.begin()+1, path.end());
        return m_root.get_from_path(subpath);
    }

} // namespace engine
