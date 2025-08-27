#ifndef ENGINE_SCENE_NODE_VIEWPORT_HPP
#define ENGINE_SCENE_NODE_VIEWPORT_HPP

#include <engine/scene/renderer.hpp>
#include <GAL/framebuffer.hpp>
#include "camera.hpp"

namespace engine {
    using framebuffer = gal::framebuffer<rc<gal::texture>>;


    // a viewport node is essentially an fbo to which its descendants should be rendered.
    // the internal fbo texture's size can be kept consistent to the resolution it should
    // be rendered to (through the use of output_resolution_changed).
    class viewport {
        //members are mutable because output_resolution_changed does not fundamentally change what they are and it needs to be const
        mutable framebuffer m_fbo;
        std::optional<glm::vec2> m_dynamic_size_relative_to_output;
        std::optional<camera> m_active_camera;
    public:
        //note: the postfx material at this stage contains a null pointer to a texture.
        viewport(framebuffer fbo, std::optional<glm::vec2> dynamic_size_relative_to_output = std::nullopt);
        viewport(glm::vec2 dynamic_size_relative_to_output);
        viewport(viewport&& o) = default;

        explicit viewport(const viewport& o);

        framebuffer& fbo();
        const framebuffer& fbo() const;
        std::optional<glm::vec2> dynamic_size_relative_to_output() const;

        void bind_draw() const;

        //can be set to null
        void set_active_camera(const std::optional<camera>& c);
        //can return null
        const std::optional<camera>& get_active_camera() const;

        /* even after resizing, texture pointers stay valid, since the texture is resized in place.
         *
         * note: it is recommended to resize viewports sparingly, since it requires allocating a new texture and destroying the previous;
         * even downsizing the viewport poses the same problem
        */
        void output_resolution_changed(glm::ivec2 native_resolution) const;

        void operator=(viewport&& o);
    };
}

#endif // ENGINE_SCENE_NODE_VIEWPORT_HPP
