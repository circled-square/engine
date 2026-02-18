#ifndef GAL_IMAGE_HPP
#define GAL_IMAGE_HPP

#include <GAL/api_macro.hpp>

namespace gal {
    struct image {
        int w, h, channels;
        void* buffer;

        GAL_API image(const char* filename);
        GAL_API image(image&& o);

        GAL_API ~image();
    };
}

#endif //GAL_IMAGE_HPP
