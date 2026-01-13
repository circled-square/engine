#include <GAL/init.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h> // glad needs glfwGetProcAddress to initialize
#include <format>
#include <stdexcept>
#include <slogga/log.hpp>
#include <iostream>

namespace gal {
    void handle_errors(GLenum source, GLenum type, GLuint msg_id, GLenum severity, int len, const char *msg, const void *user_param);

    static void initialize_error_handling() {
        glDebugMessageCallback(handle_errors, nullptr); // define a debug message (errors, warnings) callback
        glEnable(GL_DEBUG_OUTPUT); // openGL will send debug info to the callback we gave it
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // it will do so synchronously (should make it easier to find the problem with a debugger)
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,true); // it will match any type ({2}) of msg of any severity ({3}) from any ({1}) source. ({6}=false would make those errors be ignored; {4} and {5} can be used to define specific error ids to ignore/listen to)

        struct ignored_error_t { GLenum src, type; GLuint id; };
        ignored_error_t ignored_errors[] = {
                {GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, 131185 },// "Buffer object will use VIDEO memory as the source for buffer object operations"
                {GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, 131169 }, // "Framebuffer detailed info: The driver allocated storage for renderbuffer {}."
        };

        for(auto& err : ignored_errors)
            glDebugMessageControl(err.src, err.type, GL_DONT_CARE, 1, &err.id, false);
    }

    void initialize_opengl() {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
            throw std::runtime_error("GLAD failed to initialize!");

        initialize_error_handling();
    }

    void handle_errors(GLenum source, GLenum type, GLuint msg_id, GLenum severity, int len, const char *msg, const void *user_param) {
        const char *source_str =
                source == GL_DEBUG_SOURCE_API ? "API" :
                source == GL_DEBUG_SOURCE_WINDOW_SYSTEM ? "window system" :
                source == GL_DEBUG_SOURCE_SHADER_COMPILER ? "shader compiler" :
                source == GL_DEBUG_SOURCE_THIRD_PARTY ? "third party" :
                source == GL_DEBUG_SOURCE_APPLICATION ? "source application" :
                source == GL_DEBUG_SOURCE_OTHER ? "other" : "(unknown error source)";


        const char *type_str =
                type == GL_DEBUG_TYPE_ERROR ? "error" :
                type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "deprecated behavior warning" :
                type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ? "undefined behavior warning" :
                type == GL_DEBUG_TYPE_PORTABILITY ? "portability issue warning" :
                type == GL_DEBUG_TYPE_PERFORMANCE ? "performance warning" :
                type == GL_DEBUG_TYPE_MARKER ? "marker type error" :
                type == GL_DEBUG_TYPE_PUSH_GROUP ? "push group type error" :
                type == GL_DEBUG_TYPE_POP_GROUP ? "pop group type error" :
                type == GL_DEBUG_TYPE_OTHER ? "other type error" : "(unknown error type)";

        const char *severity_str =
                severity == GL_DEBUG_SEVERITY_LOW ? "low severity" :
                severity == GL_DEBUG_SEVERITY_MEDIUM ? "medium severity" :
                severity == GL_DEBUG_SEVERITY_HIGH ? "high severity" :
                severity == GL_DEBUG_SEVERITY_NOTIFICATION ? "notification" : "(unknown error severity)";

        // std::cerr << severity_str << "\t" << source_str << "\t" << type_str << std::endl;
        slogga::stdout_log.error("OpenGL error: [{} {}] source: {}, id: {}, msg: \"{}\"", severity_str, type_str, source_str, msg_id, msg);
    }
}
