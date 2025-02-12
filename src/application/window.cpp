#include <engine/application/window.hpp>
#include <glad/glad.h> // this MUST be included before glfw, so this file will include it here even though it does not need to
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <format>
#include <source_location>
#include <slogga/log.hpp>
#include <slogga/asserts.hpp>

namespace engine::window {
    inline auto throw_on_error(auto res, window_exception::code err) {
        if(!res) throw window_exception(err);
        return res;
    }

    std::int64_t window::window_count = 0;

    window::window(glm::ivec2 res, const std::string& title, creation_hints hints) {
        //Initialize the library
        if(!window_count)
            throw_on_error(glfwInit(), window_exception::code::BACKEND_INIT);

        // Create a windowed mode window and its OpenGL context
        GLFWmonitor* monitor = throw_on_error(glfwGetPrimaryMonitor(), window_exception::code::MONITOR);
        const GLFWvidmode* mode = throw_on_error(glfwGetVideoMode(monitor), window_exception::code::VIDEO_MODE);

        glfwWindowHint(GLFW_RED_BITS,     mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS,   mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,    mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        if(hints & creation_hints::MAXIMIZED)
            glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        /*
        //taken care of by glad's generator, only causes problems if done alongside it
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        */

        if(hints & creation_hints::FULLSCREEN)
            m_window_ptr = glfwCreateWindow(mode->width, mode->height, title.c_str(), monitor, nullptr);
        else
            m_window_ptr = glfwCreateWindow(res.x, res.y, title.c_str(), nullptr, nullptr);

        throw_on_error(m_window_ptr, window_exception::code::WINDOW_CREATION);

        glfwMakeContextCurrent(m_window_ptr);
        glfwSwapInterval(0); // Disable vsync: not locking fps gives us better perspective on the performance of the application during development



        window::window_count++;
    }

    window::~window() {
        glfwDestroyWindow(m_window_ptr);
        window::window_count--;

        //if no windows are left terminate glfw
        if(!window::window_count)
            glfwTerminate();

        m_window_ptr = nullptr;
    }

    void window::capture_mouse_cursor() {
        glfwSetInputMode(m_window_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ENSURES(is_mouse_cursor_captured());
    }

    void window::uncapture_mouse_cursor() {
        glfwSetInputMode(m_window_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ENSURES(!is_mouse_cursor_captured());
    }

    bool window::is_mouse_cursor_captured() { return glfwGetInputMode(m_window_ptr, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }

    bool window::should_close() { return glfwWindowShouldClose(m_window_ptr); }

    void window::swap_buffers() { glfwSwapBuffers(m_window_ptr); }

    void window::make_ctx_current() { glfwMakeContextCurrent(m_window_ptr); }

    bool window::get_key(int key) { return glfwGetKey(m_window_ptr, key) != GLFW_RELEASE; }

    bool window::get_mouse_btn(int btn) { return glfwGetMouseButton(m_window_ptr, btn) != GLFW_RELEASE; }

    void window::set_resize_cb(GLFWwindowsizefun f) { glfwSetWindowSizeCallback(m_window_ptr, f); }

    void window::set_key_cb(GLFWkeyfun f) { glfwSetKeyCallback(m_window_ptr, f); }

    void window::set_mouse_cb(GLFWcursorposfun pf, GLFWmousebuttonfun bf) {
        glfwSetCursorPosCallback(m_window_ptr, pf);
        glfwSetMouseButtonCallback(m_window_ptr, bf);
    }

    void window::set_user_ptr(void *p) { glfwSetWindowUserPointer(m_window_ptr, p); }

    void* window::get_user_ptr(GLFWwindow *w) { return glfwGetWindowUserPointer(w); }

    void window::poll_events() { glfwPollEvents(); }

    glm::dvec2 window::get_cursor_pos() {
        glm::dvec2 ret;
        glfwGetCursorPos(m_window_ptr, &ret.x, &ret.y);
        return ret;
    }

    glm::ivec2 window::get_framebuf_size() const {
        glm::ivec2 ret;
        glfwGetFramebufferSize(m_window_ptr, &ret.x, &ret.y);
        return ret;
    }

    namespace key_codes {
        const int MOUSE_1 = GLFW_MOUSE_BUTTON_1;
        const int MOUSE_2 = GLFW_MOUSE_BUTTON_2;
        const int MOUSE_3 = GLFW_MOUSE_BUTTON_3;
        const int MOUSE_4 = GLFW_MOUSE_BUTTON_4;
        const int MOUSE_5 = GLFW_MOUSE_BUTTON_5;
        const int MOUSE_6 = GLFW_MOUSE_BUTTON_6;
        const int MOUSE_7 = GLFW_MOUSE_BUTTON_7;
        const int MOUSE_8 = GLFW_MOUSE_BUTTON_8;
        const int MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT;
        const int MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;
        const int MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_RIGHT;

        const int SPACE = GLFW_KEY_SPACE;
        const int APOSTROPHE = GLFW_KEY_APOSTROPHE;
        const int COMMA = GLFW_KEY_COMMA;
        const int MINUS = GLFW_KEY_MINUS;
        const int PERIOD = GLFW_KEY_PERIOD;
        const int SLASH = GLFW_KEY_SLASH;
        const int _0 = GLFW_KEY_0;
        const int _1 = GLFW_KEY_1;
        const int _2 = GLFW_KEY_2;
        const int _3 = GLFW_KEY_3;
        const int _4 = GLFW_KEY_4;
        const int _5 = GLFW_KEY_5;
        const int _6 = GLFW_KEY_6;
        const int _7 = GLFW_KEY_7;
        const int _8 = GLFW_KEY_8;
        const int _9 = GLFW_KEY_9;
        const int SEMICOLON = GLFW_KEY_SEMICOLON;
        const int EQUAL = GLFW_KEY_EQUAL;
        const int A = GLFW_KEY_A;
        const int B = GLFW_KEY_B;
        const int C = GLFW_KEY_C;
        const int D = GLFW_KEY_D;
        const int E = GLFW_KEY_E;
        const int F = GLFW_KEY_F;
        const int G = GLFW_KEY_G;
        const int H = GLFW_KEY_H;
        const int I = GLFW_KEY_I;
        const int J = GLFW_KEY_J;
        const int K = GLFW_KEY_K;
        const int L = GLFW_KEY_L;
        const int M = GLFW_KEY_M;
        const int N = GLFW_KEY_N;
        const int O = GLFW_KEY_O;
        const int P = GLFW_KEY_P;
        const int Q = GLFW_KEY_Q;
        const int R = GLFW_KEY_R;
        const int S = GLFW_KEY_S;
        const int T = GLFW_KEY_T;
        const int U = GLFW_KEY_U;
        const int V = GLFW_KEY_V;
        const int W = GLFW_KEY_W;
        const int X = GLFW_KEY_X;
        const int Y = GLFW_KEY_Y;
        const int Z = GLFW_KEY_Z;
        const int LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET;
        const int BACKSLASH = GLFW_KEY_BACKSLASH;
        const int RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET;
        const int GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT;
        const int WORLD_1 = GLFW_KEY_WORLD_1;
        const int WORLD_2 = GLFW_KEY_WORLD_2;
        const int ESCAPE = GLFW_KEY_ESCAPE;
        const int ENTER = GLFW_KEY_ENTER;
        const int TAB = GLFW_KEY_TAB;
        const int BACKSPACE = GLFW_KEY_BACKSPACE;
        const int INSERT = GLFW_KEY_INSERT;
        const int DELETE = GLFW_KEY_DELETE;
        const int RIGHT = GLFW_KEY_RIGHT;
        const int LEFT = GLFW_KEY_LEFT;
        const int DOWN = GLFW_KEY_DOWN;
        const int UP = GLFW_KEY_UP;
        const int PAGE_UP = GLFW_KEY_PAGE_UP;
        const int PAGE_DOWN = GLFW_KEY_PAGE_DOWN;
        const int HOME = GLFW_KEY_HOME;
        const int END = GLFW_KEY_END;
        const int CAPS_LOCK = GLFW_KEY_CAPS_LOCK;
        const int SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK;
        const int NUM_LOCK = GLFW_KEY_NUM_LOCK;
        const int PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN;
        const int PAUSE = GLFW_KEY_PAUSE;
        const int F1 = GLFW_KEY_F1;
        const int F2 = GLFW_KEY_F2;
        const int F3 = GLFW_KEY_F3;
        const int F4 = GLFW_KEY_F4;
        const int F5 = GLFW_KEY_F5;
        const int F6 = GLFW_KEY_F6;
        const int F7 = GLFW_KEY_F7;
        const int F8 = GLFW_KEY_F8;
        const int F9 = GLFW_KEY_F9;
        const int F10 = GLFW_KEY_F10;
        const int F11 = GLFW_KEY_F11;
        const int F12 = GLFW_KEY_F12;
        const int F13 = GLFW_KEY_F13;
        const int F14 = GLFW_KEY_F14;
        const int F15 = GLFW_KEY_F15;
        const int F16 = GLFW_KEY_F16;
        const int F17 = GLFW_KEY_F17;
        const int F18 = GLFW_KEY_F18;
        const int F19 = GLFW_KEY_F19;
        const int F20 = GLFW_KEY_F20;
        const int F21 = GLFW_KEY_F21;
        const int F22 = GLFW_KEY_F22;
        const int F23 = GLFW_KEY_F23;
        const int F24 = GLFW_KEY_F24;
        const int F25 = GLFW_KEY_F25;
        const int KP_0 = GLFW_KEY_KP_0;
        const int KP_1 = GLFW_KEY_KP_1;
        const int KP_2 = GLFW_KEY_KP_2;
        const int KP_3 = GLFW_KEY_KP_3;
        const int KP_4 = GLFW_KEY_KP_4;
        const int KP_5 = GLFW_KEY_KP_5;
        const int KP_6 = GLFW_KEY_KP_6;
        const int KP_7 = GLFW_KEY_KP_7;
        const int KP_8 = GLFW_KEY_KP_8;
        const int KP_9 = GLFW_KEY_KP_9;
        const int KP_DECIMAL = GLFW_KEY_KP_DECIMAL;
        const int KP_DIVIDE = GLFW_KEY_KP_DIVIDE;
        const int KP_MULTIPLY = GLFW_KEY_KP_MULTIPLY;
        const int KP_SUBTRACT = GLFW_KEY_KP_SUBTRACT;
        const int KP_ADD = GLFW_KEY_KP_ADD;
        const int KP_ENTER = GLFW_KEY_KP_ENTER;
        const int KP_EQUAL = GLFW_KEY_KP_EQUAL;
        const int LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
        const int LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL;
        const int LEFT_ALT = GLFW_KEY_LEFT_ALT;
        const int LEFT_SUPER = GLFW_KEY_LEFT_SUPER;
        const int RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT;
        const int RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL;
        const int RIGHT_ALT = GLFW_KEY_RIGHT_ALT;
        const int RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER;
        const int MENU = GLFW_KEY_MENU;
        const int LAST = GLFW_KEY_LAST;
    }

    namespace modkey_codes {
        const int SHIFT = GLFW_MOD_SHIFT;
        const int CONTROL = GLFW_MOD_CONTROL;
        const int ALT = GLFW_MOD_ALT;
        const int SUPER = GLFW_MOD_SUPER;
        const int CAPS_LOCK = GLFW_MOD_CAPS_LOCK;
        const int NUM_LOCK = GLFW_MOD_NUM_LOCK;
    }

    namespace key_action_codes {
        const int RELEASE = GLFW_RELEASE;
        const int PRESS = GLFW_PRESS;
        const int REPEAT = GLFW_REPEAT;
    }

} // namespace glfw
