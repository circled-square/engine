#ifndef GAL_IMAGE_HPP
#define GAL_IMAGE_HPP

#include <GAL/api_macro.hpp>

namespace gal {
    struct image {
        int w, h, channels; //NOLINT(cppcoreguidelines-use-default-member-init)
        void* buffer; //NOLINT(cppcoreguidelines-use-default-member-init)

        GAL_API explicit image(const char* filename);
        GAL_API image(image&& o) noexcept;
        image(const image&) = delete;
        image& operator=(image&&) = delete;
        image& operator=(const image&) = delete;

        GAL_API ~image();
    };
}

#endif //GAL_IMAGE_HPP
