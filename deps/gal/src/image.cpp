#include <GAL/image.hpp>
#include <stb_image.h>
#include <cassert>
#include <stdexcept>
#include <format>

namespace gal {
    image::image(const char* filename) {
        stbi_set_flip_vertically_on_load(1);
        buffer = stbi_load(filename, &w, &h, &channels, 4);
        if(buffer == nullptr)
            throw std::runtime_error(std::format("failed opening image at path {}", filename));
        stbi_set_flip_vertically_on_load(0);
    }

    image::image(image&& o) {
        w = o.w;
        h = o.h;
        channels = o.channels;
        buffer = o.buffer;

        o.buffer = nullptr;
    }

    image::~image() {
        if(buffer)
            stbi_image_free(buffer);
        buffer = nullptr;
    }
}
