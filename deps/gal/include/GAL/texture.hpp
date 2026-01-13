#ifndef GAL_TEXTURE_HPP
#define GAL_TEXTURE_HPP

#include "image.hpp"
#include <GAL/types.hpp>
#include <random>

namespace gal {
    namespace internal {
        class framebuffer; //only to declare as friend of texture
    }
    class texture {
        friend class internal::framebuffer;
        uint m_texture_id;
        glm::ivec2 m_res;
        int m_components;

        //necessary for creating a null texture
        explicit texture(std::nullptr_t);

      public:
        struct specification {
            glm::ivec2 res;
            int components = 4;

            const void* data = nullptr;
            int alignment = 4;

            bool repeat_wrap = false;
            GLenum filter_method = GL_NEAREST;
            //TODO: specification is missing a parameter to allow the user to have more than 8 bit depth for each component
        };
        texture(const specification& spec);
        texture(const image& image);
        texture(texture&& o);
        texture& operator=(texture&& o);
        ~texture();

        static texture null();
        bool is_null();

        void set_texture_data(const void* buffer, int alignment = 4);

        void bind(uint slot) const;

        int width () const;
        int height() const;
        glm::ivec2 resolution() const;

        uint get_gl_id();

        //returns a texture containing random noise
        static texture noise(glm::ivec2 res, char components = 4);

        //returns a texture without setting any data for it
        static texture empty(glm::ivec2 res, char components = 4);
    };
}


#endif //GAL_TEXTURE_HPP
