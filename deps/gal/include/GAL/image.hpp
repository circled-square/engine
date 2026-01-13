#ifndef GAL_IMAGE_HPP
#define GAL_IMAGE_HPP


namespace gal {
    struct image {
        int w, h, channels;
        void* buffer;

        image(const char* filename);
        image(image&& o);

        ~image();
    };

}

#endif //GAL_IMAGE_HPP
