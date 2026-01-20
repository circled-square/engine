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
        enum class filter_method : bool { nearest, linear };
        struct specification {
            glm::ivec2 res;
            int components = 4;

            // TODO: substitute void* for std::span<std::byte>
            const void* data = nullptr;
            int alignment = 4;
            bool enable_mipmaps = true;

            //TODO: specification is missing a parameter to allow the user to have more than 8 bit depth for each component

            //TODO: the following settings should be sampler settings instead of texture settings
            bool repeat_wrap = false;
            texture::filter_method filter_method = texture::filter_method::nearest;
            texture::filter_method mipmap_filter_method = texture::filter_method::linear;
            bool enable_anisotropic_filtering = true; // ignored if mipmap_filter_method == nullopt
            float max_anisotropy = 16.f; // ignored unless anisotropic_filtering == true

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

        //returns a texture containing pseudo-random noise
        static texture noise(glm::ivec2 res, char components = 4);

        //returns a texture without setting any data for it
        static texture empty(glm::ivec2 res, char components = 4);
    };
}


#endif //GAL_TEXTURE_HPP
