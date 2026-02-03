#ifndef ENGINE_APPLICATION_WINDOW_HPP
#define ENGINE_APPLICATION_WINDOW_HPP
/*
engine::window::window is a RAII and OO wrapper for glfw windows, thread unsafe because glfw is (unfortunately).
Construction and destruction may only be invoked in the main thread but some other functions *might* be
callable from other threads.
*/
#include <string>
#include <cstdint>
#include <glm/glm.hpp>

struct GLFWwindow; // forward declaration to avoid needlessly including glfw everywhere

namespace engine::window {
    struct hints {
        bool fullscreen = false;
        bool maximised = false;
        bool vsync = false; // vsync disabled by default: not locking fps gives us better perspective on the performance of the application during development
    };

    class window {
        static std::int64_t window_count;
        GLFWwindow* m_window_ptr;
    public:
        window() = delete;
        window(window&&) = delete;
        window(const window&) = delete;

        window(glm::ivec2 res, const std::string& title, hints window_hints = {});
        ~window();

        bool should_close();
        void swap_buffers();
        void make_ctx_current();
        static void poll_events();

        void capture_mouse_cursor();
        void uncapture_mouse_cursor();
        bool is_mouse_cursor_captured();

        bool get_key(int key);
        bool get_mouse_btn(int btn);
        glm::dvec2 get_cursor_pos();

        glm::ivec2 get_framebuf_size() const;

        void set_vsync(bool value);


        // set callbacks
        using resize_cb_t = void (*)(GLFWwindow*, int, int);
        void set_resize_cb(resize_cb_t f);

        using key_cb_t = void (*)(GLFWwindow*, int, int, int, int);
        void set_key_cb(key_cb_t f);

        using mouse_position_cb_t = void (*)(GLFWwindow*, double, double);
        using mouse_button_cb_t = void (*)(GLFWwindow*, int, int, int);
        void set_mouse_cb(mouse_position_cb_t, mouse_button_cb_t);


        void set_user_ptr(void* p);
        static void* get_user_ptr(GLFWwindow* w);


        //access and conversion of the internal GLFWwindow*
        operator GLFWwindow* () { return  m_window_ptr; }
        operator bool     () { return  m_window_ptr; }
        bool operator!    () { return !m_window_ptr; }

        using generic_fn_ptr_t = void(*)();
        using opengl_function_loader_t = generic_fn_ptr_t(*)(const char*);

        opengl_function_loader_t get_opengl_function_loader();
    };

    struct window_exception : public std::exception {
        enum class code {
            BACKEND_INIT,    //GLFW3 was unable to initialize
            MONITOR,         //GLFW3 was unable to find primary monitor
            VIDEO_MODE,      //GLFW3 was unable to get video mode
            WINDOW_CREATION, //GLFW3 was unable to create the window
        };
        code error;

        explicit window_exception(window_exception::code err);

        virtual const char* what() const noexcept;
    };

    namespace key_codes {
        extern const int MOUSE_1;
        extern const int MOUSE_2;
        extern const int MOUSE_3;
        extern const int MOUSE_4;
        extern const int MOUSE_5;
        extern const int MOUSE_6;
        extern const int MOUSE_7;
        extern const int MOUSE_8;
        extern const int MOUSE_LEFT;
        extern const int MOUSE_RIGHT;
        extern const int MOUSE_MIDDLE;

        extern const int SPACE;
        extern const int APOSTROPHE;
        extern const int COMMA;
        extern const int MINUS;
        extern const int PERIOD;
        extern const int SLASH;
        extern const int _0;
        extern const int _1;
        extern const int _2;
        extern const int _3;
        extern const int _4;
        extern const int _5;
        extern const int _6;
        extern const int _7;
        extern const int _8;
        extern const int _9;
        extern const int SEMICOLON;
        extern const int EQUAL;
        extern const int A;
        extern const int B;
        extern const int C;
        extern const int D;
        extern const int E;
        extern const int F;
        extern const int G;
        extern const int H;
        extern const int I;
        extern const int J;
        extern const int K;
        extern const int L;
        extern const int M;
        extern const int N;
        extern const int O;
        extern const int P;
        extern const int Q;
        extern const int R;
        extern const int S;
        extern const int T;
        extern const int U;
        extern const int V;
        extern const int W;
        extern const int X;
        extern const int Y;
        extern const int Z;
        extern const int LEFT_BRACKET;
        extern const int BACKSLASH;
        extern const int RIGHT_BRACKET;
        extern const int GRAVE_ACCENT;
        extern const int WORLD_1;
        extern const int WORLD_2;
        extern const int ESCAPE;
        extern const int ENTER;
        extern const int TAB;
        extern const int BACKSPACE;
        extern const int INSERT;
        extern const int DELETE;
        extern const int RIGHT;
        extern const int LEFT;
        extern const int DOWN;
        extern const int UP;
        extern const int PAGE_UP;
        extern const int PAGE_DOWN;
        extern const int HOME;
        extern const int END;
        extern const int CAPS_LOCK;
        extern const int SCROLL_LOCK;
        extern const int NUM_LOCK;
        extern const int PRINT_SCREEN;
        extern const int PAUSE;
        extern const int F1;
        extern const int F2;
        extern const int F3;
        extern const int F4;
        extern const int F5;
        extern const int F6;
        extern const int F7;
        extern const int F8;
        extern const int F9;
        extern const int F10;
        extern const int F11;
        extern const int F12;
        extern const int F13;
        extern const int F14;
        extern const int F15;
        extern const int F16;
        extern const int F17;
        extern const int F18;
        extern const int F19;
        extern const int F20;
        extern const int F21;
        extern const int F22;
        extern const int F23;
        extern const int F24;
        extern const int F25;
        extern const int KP_0;
        extern const int KP_1;
        extern const int KP_2;
        extern const int KP_3;
        extern const int KP_4;
        extern const int KP_5;
        extern const int KP_6;
        extern const int KP_7;
        extern const int KP_8;
        extern const int KP_9;
        extern const int KP_DECIMAL;
        extern const int KP_DIVIDE;
        extern const int KP_MULTIPLY;
        extern const int KP_SUBTRACT;
        extern const int KP_ADD;
        extern const int KP_ENTER;
        extern const int KP_EQUAL;
        extern const int LEFT_SHIFT;
        extern const int LEFT_CONTROL;
        extern const int LEFT_ALT;
        extern const int LEFT_SUPER;
        extern const int RIGHT_SHIFT;
        extern const int RIGHT_CONTROL;
        extern const int RIGHT_ALT;
        extern const int RIGHT_SUPER;
        extern const int MENU;
        extern const int LAST;
    }

    namespace modkey_codes {
        extern const int SHIFT;
        extern const int CONTROL;
        extern const int ALT;
        extern const int SUPER;
        extern const int CAPS_LOCK;
        extern const int NUM_LOCK;
    }

    namespace key_action_codes {
        extern const int RELEASE;
        extern const int PRESS;
        extern const int REPEAT;
    }

}

#endif //ENGINE_APPLICATION_WINDOW_HPP
