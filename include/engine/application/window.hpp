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
#include <engine/utils/api_macro.hpp>

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
        ENGINE_API extern const int MOUSE_1;
        ENGINE_API extern const int MOUSE_2;
        ENGINE_API extern const int MOUSE_3;
        ENGINE_API extern const int MOUSE_4;
        ENGINE_API extern const int MOUSE_5;
        ENGINE_API extern const int MOUSE_6;
        ENGINE_API extern const int MOUSE_7;
        ENGINE_API extern const int MOUSE_8;
        ENGINE_API extern const int MOUSE_LEFT;
        ENGINE_API extern const int MOUSE_RIGHT;
        ENGINE_API extern const int MOUSE_MIDDLE;

        ENGINE_API extern const int SPACE;
        ENGINE_API extern const int APOSTROPHE;
        ENGINE_API extern const int COMMA;
        ENGINE_API extern const int MINUS;
        ENGINE_API extern const int PERIOD;
        ENGINE_API extern const int SLASH;
        ENGINE_API extern const int _0;
        ENGINE_API extern const int _1;
        ENGINE_API extern const int _2;
        ENGINE_API extern const int _3;
        ENGINE_API extern const int _4;
        ENGINE_API extern const int _5;
        ENGINE_API extern const int _6;
        ENGINE_API extern const int _7;
        ENGINE_API extern const int _8;
        ENGINE_API extern const int _9;
        ENGINE_API extern const int SEMICOLON;
        ENGINE_API extern const int EQUAL;
        ENGINE_API extern const int A;
        ENGINE_API extern const int B;
        ENGINE_API extern const int C;
        ENGINE_API extern const int D;
        ENGINE_API extern const int E;
        ENGINE_API extern const int F;
        ENGINE_API extern const int G;
        ENGINE_API extern const int H;
        ENGINE_API extern const int I;
        ENGINE_API extern const int J;
        ENGINE_API extern const int K;
        ENGINE_API extern const int L;
        ENGINE_API extern const int M;
        ENGINE_API extern const int N;
        ENGINE_API extern const int O;
        ENGINE_API extern const int P;
        ENGINE_API extern const int Q;
        ENGINE_API extern const int R;
        ENGINE_API extern const int S;
        ENGINE_API extern const int T;
        ENGINE_API extern const int U;
        ENGINE_API extern const int V;
        ENGINE_API extern const int W;
        ENGINE_API extern const int X;
        ENGINE_API extern const int Y;
        ENGINE_API extern const int Z;
        ENGINE_API extern const int LEFT_BRACKET;
        ENGINE_API extern const int BACKSLASH;
        ENGINE_API extern const int RIGHT_BRACKET;
        ENGINE_API extern const int GRAVE_ACCENT;
        ENGINE_API extern const int WORLD_1;
        ENGINE_API extern const int WORLD_2;
        ENGINE_API extern const int ESCAPE;
        ENGINE_API extern const int ENTER;
        ENGINE_API extern const int TAB;
        ENGINE_API extern const int BACKSPACE;
        ENGINE_API extern const int INSERT;
        ENGINE_API extern const int DELETE;
        ENGINE_API extern const int RIGHT;
        ENGINE_API extern const int LEFT;
        ENGINE_API extern const int DOWN;
        ENGINE_API extern const int UP;
        ENGINE_API extern const int PAGE_UP;
        ENGINE_API extern const int PAGE_DOWN;
        ENGINE_API extern const int HOME;
        ENGINE_API extern const int END;
        ENGINE_API extern const int CAPS_LOCK;
        ENGINE_API extern const int SCROLL_LOCK;
        ENGINE_API extern const int NUM_LOCK;
        ENGINE_API extern const int PRINT_SCREEN;
        ENGINE_API extern const int PAUSE;
        ENGINE_API extern const int F1;
        ENGINE_API extern const int F2;
        ENGINE_API extern const int F3;
        ENGINE_API extern const int F4;
        ENGINE_API extern const int F5;
        ENGINE_API extern const int F6;
        ENGINE_API extern const int F7;
        ENGINE_API extern const int F8;
        ENGINE_API extern const int F9;
        ENGINE_API extern const int F10;
        ENGINE_API extern const int F11;
        ENGINE_API extern const int F12;
        ENGINE_API extern const int F13;
        ENGINE_API extern const int F14;
        ENGINE_API extern const int F15;
        ENGINE_API extern const int F16;
        ENGINE_API extern const int F17;
        ENGINE_API extern const int F18;
        ENGINE_API extern const int F19;
        ENGINE_API extern const int F20;
        ENGINE_API extern const int F21;
        ENGINE_API extern const int F22;
        ENGINE_API extern const int F23;
        ENGINE_API extern const int F24;
        ENGINE_API extern const int F25;
        ENGINE_API extern const int KP_0;
        ENGINE_API extern const int KP_1;
        ENGINE_API extern const int KP_2;
        ENGINE_API extern const int KP_3;
        ENGINE_API extern const int KP_4;
        ENGINE_API extern const int KP_5;
        ENGINE_API extern const int KP_6;
        ENGINE_API extern const int KP_7;
        ENGINE_API extern const int KP_8;
        ENGINE_API extern const int KP_9;
        ENGINE_API extern const int KP_DECIMAL;
        ENGINE_API extern const int KP_DIVIDE;
        ENGINE_API extern const int KP_MULTIPLY;
        ENGINE_API extern const int KP_SUBTRACT;
        ENGINE_API extern const int KP_ADD;
        ENGINE_API extern const int KP_ENTER;
        ENGINE_API extern const int KP_EQUAL;
        ENGINE_API extern const int LEFT_SHIFT;
        ENGINE_API extern const int LEFT_CONTROL;
        ENGINE_API extern const int LEFT_ALT;
        ENGINE_API extern const int LEFT_SUPER;
        ENGINE_API extern const int RIGHT_SHIFT;
        ENGINE_API extern const int RIGHT_CONTROL;
        ENGINE_API extern const int RIGHT_ALT;
        ENGINE_API extern const int RIGHT_SUPER;
        ENGINE_API extern const int MENU;
        ENGINE_API extern const int LAST;
    }

    namespace modkey_codes {
        ENGINE_API extern const int SHIFT;
        ENGINE_API extern const int CONTROL;
        ENGINE_API extern const int ALT;
        ENGINE_API extern const int SUPER;
        ENGINE_API extern const int CAPS_LOCK;
        ENGINE_API extern const int NUM_LOCK;
    }

    namespace key_action_codes {
        ENGINE_API extern const int RELEASE;
        ENGINE_API extern const int PRESS;
        ENGINE_API extern const int REPEAT;
    }

}

#endif //ENGINE_APPLICATION_WINDOW_HPP
