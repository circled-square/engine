#include <GAL/texture.hpp>
#include <glad/glad.h>

namespace gal {
    texture::texture(const specification& spec)
            : m_res(spec.res), m_components(spec.components) {

        assert(m_components >= 1 && m_components <= 4);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_id);

        //obligatory parameters
        glTextureParameteri(m_texture_id, GL_TEXTURE_MIN_FILTER, spec.filter_method);
        glTextureParameteri(m_texture_id, GL_TEXTURE_MAG_FILTER, spec.filter_method);

        int wrap_mode = spec.repeat_wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_S, wrap_mode);
        glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_T, wrap_mode);

        //init the immutable storage
        GLenum internal_format =
            m_components == 1 ? GL_R8 :
            m_components == 2 ? GL_RG8 :
            m_components == 3 ? GL_RGB8 :
            GL_RGBA8;
        glTextureStorage2D(m_texture_id, 1, internal_format, m_res.x, m_res.y);

        //write on the immutable storage
        if(spec.data) {
            set_texture_data(spec.data, spec.alignment);
        }
    }


    static texture::specification specification_from_image(const image& img) {
        texture::specification spec;
        spec.res = { img.w, img.h };
        spec.components = 4;
        spec.data = img.buffer;

        return spec;
    }

    texture::texture(const image& image) : texture::texture(specification_from_image(image)) {}

    texture::texture(texture&& o) {
        m_texture_id = o.m_texture_id;
        o.m_texture_id = 0;
        m_res = o.m_res;
        m_components = o.m_components;
    }

    texture& texture::operator=(texture&& o) {
        gal::texture let_this_be_destroyed(std::move(*this));

        m_texture_id = o.m_texture_id;
        o.m_texture_id = 0;
        m_res = o.m_res;
        m_components = o.m_components;

        return *this;
    }
    texture::~texture() {
        glDeleteTextures(1, &m_texture_id);
    }

    void texture::set_texture_data(const void *buffer, int alignment) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

        uint format =
            m_components == 1 ? GL_RED :
            m_components == 2 ? GL_RG :
            m_components == 3 ? GL_RGB : GL_RGBA;

        if(buffer != nullptr)
            glTextureSubImage2D(m_texture_id, 0, 0, 0, m_res.x, m_res.y, format, GL_UNSIGNED_BYTE, buffer);
    }

    void texture::bind(uint slot) const {
        glBindTextureUnit(slot, m_texture_id);
    }

    int texture::width() const { return m_res.x; }
    int texture::height() const { return m_res.y; }
    glm::ivec2 texture::resolution() const { return m_res; }

    uint texture::get_gl_id() { return m_texture_id; }

    texture texture::noise(glm::ivec2 res, char components) {
        std::random_device rand_dev;
        std::subtract_with_carry_engine<unsigned char, 8, 5, 12> rng(rand_dev());

        unsigned char* data = new unsigned char[res.x*res.y*components];

        for (int y = 0; y < res.y; y++) {
            for (int x = 0; x < res.x; x++) {
                for (char c = 0; c < components; c++) {
                    data[c + x*components + y*res.x*components] = rng();
                }
            }
        }
        specification spec = {
            .res=res, .components=components, .data=data, .alignment=1, .repeat_wrap=true,
        };

        return texture(spec);
    }

    texture texture::empty(glm::ivec2 res, char components) {
        return texture {
            specification {
                .res = res, .components = components
            }
        };
    }

    texture::texture(std::nullptr_t) : m_texture_id(0), m_res({0, 0}), m_components(0) {}

    texture texture::null() { return texture(nullptr); }

    bool texture::is_null() { return m_texture_id == 0; }
}
