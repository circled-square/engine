#include <GAL/framebuffer.hpp>
#include <glad/glad.h>
#include <unordered_map>
#include <format>


// these classes don't use OpenGL Direct State Access simply because it doesn't seem to work the way you'd expect with FBOs and renderbuffers
namespace gal {
    namespace internal {
        framebuffer::framebuffer(texture* tex) : m_fbo((unsigned)-1), m_depth_renderbuf_id((unsigned)-1), m_resolution(-1,-1) {
            glGenFramebuffers(1, &m_fbo);
            bind();

            // create depth buffer
            glGenRenderbuffers(1, &m_depth_renderbuf_id);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_renderbuf_id);

            // attach depth buffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_renderbuf_id);

            if(tex) {
                link_texture(*tex);
            }

            unbind(); // limit side effects to a minimum
        }

        framebuffer::framebuffer(framebuffer&& o): m_fbo(o.m_fbo), m_depth_renderbuf_id(o.m_depth_renderbuf_id), m_resolution(o.m_resolution) {
            o.m_fbo = o.m_depth_renderbuf_id = -1;
        }

        framebuffer::~framebuffer() {
            if(m_fbo != -1)
                glDeleteFramebuffers(1, &m_fbo);
            if(m_depth_renderbuf_id != -1)
                glDeleteRenderbuffers(1, &m_depth_renderbuf_id);
            m_fbo = m_depth_renderbuf_id = -1;
        }

        void framebuffer::link_texture(texture& tex) {
            m_resolution = tex.resolution();

            // resize the depth renderbuffer storage
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_renderbuf_id);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, tex.width(), tex.height());

            // attach the texture
            bind();
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.m_texture_id, 0);// attach the texture {3}'s mipmap {4} to framebuffer {0} at attach index {1}

            unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                framebuffer::unbind(); // limit side effects to a minimum
                throw framebuffer_construction_exception(status);
            }

            // Set the list of draw buffers.
            unsigned int draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, draw_buffers);

            unbind(); // limit side effects to a minimum
        }

        void framebuffer::bind_draw() {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        }

        void framebuffer::bind_read() const {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        }

        void framebuffer::bind() {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        }

        void framebuffer::unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        const glm::ivec2 framebuffer::resolution() const { return m_resolution; }

        framebuffer& framebuffer::operator=(framebuffer&& o) {
            this->~framebuffer();
            m_fbo = o.m_fbo;
            o.m_fbo = -1;
            m_depth_renderbuf_id = o.m_depth_renderbuf_id;
            o.m_depth_renderbuf_id = -1;
            m_resolution = o.m_resolution;

            return *this;
        }
    }

    // construction_exception
    framebuffer_construction_exception::framebuffer_construction_exception(unsigned int error_code) : m_code(error_code) {
        std::unordered_map<unsigned int, std::string> glenum_to_string = {
            { GL_FRAMEBUFFER_UNDEFINED, 						"GL_FRAMEBUFFER_UNDEFINED" },
            { GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT, 			"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" },
            { GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,  	"GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" },
            { GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER, 			"GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" },
            { GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,			"GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" },
            { GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE, 			"GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" },
            { GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,			"GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" },
            { GL_FRAMEBUFFER_UNSUPPORTED,						"GL_FRAMEBUFFER_UNSUPPORTED" }
        };

        m_what = std::format("error during framebuffer object construction: {} ({})", glenum_to_string[m_code], m_code);
    }

    unsigned int framebuffer_construction_exception::get_code() const noexcept { return m_code; }

    const char* framebuffer_construction_exception::what() const noexcept { return m_what.c_str(); }
}
