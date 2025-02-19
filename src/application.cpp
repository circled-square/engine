#include <engine/application.hpp>

#include <GAL/init.hpp>
#include <engine/resources_manager.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <chrono>
#include <slogga/log.hpp>

namespace engine {
    application::application(glm::ivec2 res, const std::string &title, int window_hints)
            : m_window(res, title, window_hints), m_active_scene(),
              m_prev_mouse_cursor_pos(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()),
              m_ignore_mouse_move_on_next_event(true)
    {
        //init opengl (after glfw)
        gal::initialize_opengl();
        //init resources_manager (after opengl)
        resources_manager::init_instance();

        slogga::stdout_log.info("{}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));


        // set window callbacks and set the user pointer for them
        m_window.set_user_ptr(this);
        m_window.set_key_cb([] (GLFWwindow* window_handle, int key, int scancode, int action, int mods) {
            if(ImGui::GetIO().WantCaptureKeyboard)
                return;

            application* app = reinterpret_cast<application*>(window::window::get_user_ptr(window_handle));

            app->m_events_this_frame.push_back(key_event_t{key, scancode, action, mods});
        });
        m_window.set_mouse_cb([] (GLFWwindow* window_handle, double xpos, double ypos) {
            if(ImGui::GetIO().WantCaptureMouse)
                return;

            application* app = reinterpret_cast<application*>(window::window::get_user_ptr(window_handle));
            glm::vec2 pos = {xpos, ypos};
            glm::vec2 movement;

            if(std::isinf(app->m_prev_mouse_cursor_pos.x)) {
                //first event is garbage unfortunately
                app->m_prev_mouse_cursor_pos = {0,0};
                return;
            } else if(app->m_ignore_mouse_move_on_next_event) {
                movement = {0,0};
                app->m_ignore_mouse_move_on_next_event = false;
            } else {
                movement = pos - app->m_prev_mouse_cursor_pos;
            }


            app->m_events_this_frame.push_back(mouse_move_event_t{pos, movement});
            app->m_prev_mouse_cursor_pos = pos;

        }, [](GLFWwindow* window_handle, int button, int action, int mods) {
            if(ImGui::GetIO().WantCaptureMouse)
                return;
            application* app = reinterpret_cast<application*>(window::window::get_user_ptr(window_handle));

            app->m_events_this_frame.push_back(key_event_t{button, -1, action, mods});
        });


        // init imgui (after setting glfw callbacks and after init of opengl)
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);// after setting callbacks, this way imgui can handle dispatching inputs to itself or to us
        ImGui_ImplOpenGL3_Init(); // after init of opengl
        ImGui::StyleColorsDark();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }

    void application::set_start_scene(rc<scene> start_scene) {
        EXPECTS(!m_active_scene && start_scene);
        m_active_scene = std::move(start_scene);
        m_active_scene->prepare();
        m_active_scene->app_channel().from_app_mut().scene_is_active = true;
    }

    application::~application() {
        m_active_scene.~rc(); // deinit this before resources_manager, or else it will be dangling by the time its destructor is called

        resources_manager::deinit_instance();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void application::run() {
        using clock = std::chrono::steady_clock;
        auto app_start_time = clock::now(); // frame_time is an offset from this
        auto last_frame_time = app_start_time;  //used to calculate the delta

        while (!m_window.should_close()) {
            auto curr_frame_time = clock::now();
            float delta = std::chrono::duration<float>(curr_frame_time - last_frame_time).count();
            float frame_time = std::chrono::duration<float>(curr_frame_time - app_start_time).count();

            // read and write to the channel to the scene
            EXPECTS(m_active_scene);
            rc<scene> scene_to_change_to = std::move(m_active_scene->app_channel().to_app().scene_to_change_to);
            if(scene_to_change_to) {
                ASSERTS(m_active_scene->app_channel().from_app_mut().scene_is_active);

                m_active_scene->app_channel().from_app_mut().scene_is_active = false;

                m_active_scene = std::move(scene_to_change_to);
                m_active_scene->app_channel().from_app_mut().scene_is_active = true;
                m_active_scene->prepare();
            }

            bool cursor_is_captured = m_window.is_mouse_cursor_captured();
            bool wants_cursor_captured = m_active_scene->app_channel().to_app().wants_mouse_cursor_captured;
            if(wants_cursor_captured && !cursor_is_captured) {
                m_window.capture_mouse_cursor();
                m_ignore_mouse_move_on_next_event = true;
                cursor_is_captured = true;
            } else if(!wants_cursor_captured && cursor_is_captured) {
                m_window.uncapture_mouse_cursor();
                m_ignore_mouse_move_on_next_event = true;
                cursor_is_captured = false;
            }


            ASSERTS(m_window.is_mouse_cursor_captured() == cursor_is_captured);
            m_active_scene->app_channel().from_app_mut().mouse_cursor_is_captured = cursor_is_captured;
            m_active_scene->app_channel().from_app_mut().framebuffer_size = m_window.get_framebuf_size();
            m_active_scene->app_channel().from_app_mut().delta = delta;
            m_active_scene->app_channel().from_app_mut().frame_time = frame_time;
            m_events_this_frame.clear();
            m_window.poll_events(); // populates m_events_this_frame
            m_active_scene->app_channel().from_app_mut().events = m_events_this_frame;

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();


            m_active_scene->update();
            m_active_scene->render();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            m_window.swap_buffers();



            last_frame_time = curr_frame_time;
        }
    }
}
