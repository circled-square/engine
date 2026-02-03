#ifndef GAL_INTERNAL_INIT_HPP
#define GAL_INTERNAL_INIT_HPP

namespace gal {
    using opengl_function_loader_t = void* (*)(const char *);
    void initialize_opengl(opengl_function_loader_t opengl_function_loader);
}

#endif //GAL_INTERNAL_INIT_HPP
