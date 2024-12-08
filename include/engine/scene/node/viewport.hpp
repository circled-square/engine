#ifndef ENGINE_SCENE_NODE_VIEWPORT_HPP
#define ENGINE_SCENE_NODE_VIEWPORT_HPP

#include <engine/material.hpp>
#include <GAL/framebuffer.hpp>

namespace engine {
    using framebuffer = gal::framebuffer<rc<gal::texture>>;

    // a viewport node is composed of the fbo to which its descendants should be rendered, and the shader to be applied when rendering the resulting texture.
    // the internal fbo texture's size is kept consistent to the resolution it should be rendered to (through the use of output_resolution_changed).
    class viewport {
        //members are mutable because output_resolution_changed does not fundamentally change what they are and it needs to be const
        mutable framebuffer m_fbo;
        mutable material m_postfx_material;
        std::optional<glm::vec2> m_dynamic_size_relative_to_output;
        const camera* m_active_camera = nullptr;
    public:
        //note: the postfx material at this stage contains a null pointer to a texture.
        viewport(framebuffer fbo, rc<const shader> postfx_shader, std::optional<glm::vec2> dynamic_size_relative_to_output = std::nullopt);
        viewport(rc<const shader> postfx_shader, glm::vec2 dynamic_size_relative_to_output);
        viewport(viewport&& o);

        explicit viewport(const viewport& o);

        framebuffer& fbo();
        const framebuffer& fbo() const;
        material& postfx_material();
        const material& postfx_material() const;
        std::optional<glm::vec2> dynamic_size_relative_to_output() const;

        void bind_draw() const;

        //can be set to null
        void set_active_camera(const camera* c);
        //can return null
        const camera* get_active_camera() const;

        // note: it is recommended to resize viewports sparingly, since it requires allocating a new texture and "leaking" to the gc the old one.
        void output_resolution_changed(glm::ivec2 native_resolution) const;

        void operator=(viewport&& o);
    };
}

#endif // ENGINE_SCENE_NODE_VIEWPORT_HPP
