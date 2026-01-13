#ifndef GAL_FRAMEBUFFER_HPP
#define GAL_FRAMEBUFFER_HPP

#include "texture.hpp"
#include <stdexcept>
#include <optional>

namespace gal {
    class framebuffer_construction_exception : std::exception {
        unsigned int m_code;
        std::string m_what;
    public:
        framebuffer_construction_exception(unsigned int error_code);
        virtual const char* what() const noexcept;
        unsigned int get_code() const noexcept;
    };

    // internal::framebuffer implements framebuffer behaviour without being a template
    namespace internal {
        class framebuffer {
            unsigned int m_fbo;
            unsigned int m_depth_renderbuf_id;
            glm::ivec2 m_resolution;

        public:
            framebuffer(const framebuffer&) = delete;

            //may throw construction_exception
            framebuffer(texture* tex = nullptr);
            framebuffer(framebuffer&& o);
            virtual ~framebuffer();

            // switches the texture this fbo is linked to to a different one
            void link_texture(texture& tex);

            void bind_draw();
            void bind_read() const;
            void bind();
            static void unbind();

            const glm::ivec2 resolution() const;

            framebuffer& operator=(framebuffer&& o);
        };
    }

    template<typename ptr_t, typename pointed_t>
    concept PointerLike = requires(ptr_t p)
    {
        { *p } -> std::convertible_to<const pointed_t&>;
    };

    template<PointerLike<texture> tex_ptr_t = std::optional<texture>>
    class framebuffer {
        tex_ptr_t m_tex;
        internal::framebuffer m_fbo;
    public:
        framebuffer(const framebuffer&) = delete;
        framebuffer() = delete;

        //may throw construction_exception
        framebuffer(framebuffer&& o) : m_tex(std::move(o.m_tex)), m_fbo(std::move(o.m_fbo)) {}
        framebuffer(tex_ptr_t tex) : m_tex(std::move(tex)), m_fbo(m_tex ? (m_tex->is_null() ? nullptr : &*m_tex) : nullptr) {}

        // switches the texture this fbo is linked to to a different one
        void link_texture(tex_ptr_t tex) {
            m_tex = std::move(tex);
            if(!m_tex->is_null())
                m_fbo.link_texture(*m_tex);
        }

        // switches the texture this fbo is linked to to a different one, replacing the previous one through this objects PointerLike
        void link_and_replace_texture(texture tex) {
            *m_tex = std::move(tex);
            if(!m_tex->is_null())
                m_fbo.link_texture(*m_tex);
        }

        void bind_draw() { return m_fbo.bind_draw(); }
        void bind_read() const { return m_fbo.bind_read(); }
        void bind() { return m_fbo.bind(); }
        static void unbind() { return internal::framebuffer::unbind(); }

        const glm::ivec2 resolution() const { return m_fbo.resolution(); }

        tex_ptr_t& get_texture() { return m_tex; }
        const tex_ptr_t& get_texture() const { return m_tex; }

        framebuffer& operator=(framebuffer&& o) {
            m_tex = std::move(o.m_tex);
            m_fbo = std::move(o.m_fbo);
            return *this;
        }
    };
}

#endif // GAL_FRAMEBUFFER_HPP
