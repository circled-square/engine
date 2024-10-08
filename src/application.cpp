#include <engine/application.hpp>

#include <GAL/init.hpp>
#include <engine/resources_manager.hpp>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <chrono>
#include <slogga/log.hpp>

#include <iostream>

namespace engine {
    application::application(glm::ivec2 res, const std::string &title, window::creation_hints hints)
            : m_window(res, title, hints), m_active_scene(),
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

            app->m_active_scene->on_key_press(key, scancode, action, mods);
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


            app->m_active_scene->on_mouse_move(pos, movement);
            app->m_prev_mouse_cursor_pos = pos;

        }, [](GLFWwindow* window_handle, int button, int action, int mods) {
            if(ImGui::GetIO().WantCaptureMouse)
                return;
            application* app = reinterpret_cast<application*>(window::window::get_user_ptr(window_handle));

            app->m_active_scene->on_key_press(button, -1, action, mods);
        });


        // init imgui (after setting glfw callbacks and after init of opengl)
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);// after setting callbacks, this way imgui can handle dispatching inputs to itself or to us
        ImGui_ImplOpenGL3_Init(); // after init of opengl
        ImGui::StyleColorsDark();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }

    application::~application() {
        m_active_scene.~rc(); // deinit this before resources_manager, or else it will be dangling by the time its destructor is called

        resources_manager::deinit_instance();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    rc<basic_scene> application::set_active_scene(rc<basic_scene> new_scene) {
        rc<basic_scene> old_active_scene = std::move(m_active_scene);
        m_active_scene = std::move(new_scene);

        if(old_active_scene) {
            EXPECTS(old_active_scene->application_channel().from_app.scene_is_active);
            old_active_scene->freeze();
            old_active_scene->application_channel().from_app.scene_is_active = false;
        }

        m_active_scene->application_channel().from_app.scene_is_active = true;
        m_active_scene->reheat();

        return old_active_scene;
    }

    void application::run() {
        using clock = std::chrono::steady_clock;
        auto app_start_time = clock::now(); // frame_time is an offset from this
        auto last_frame_time = app_start_time;  //used to calculate the delta

        while (!m_window.should_close()) {
            auto curr_frame_time = clock::now();

            // read and write to the channel with the scene
            bool cursor_is_captured = m_window.is_mouse_cursor_captured();

            if(m_active_scene->application_channel().to_app.scene_to_change_to)
                set_active_scene(std::move(m_active_scene->application_channel().to_app.scene_to_change_to));

            if(m_active_scene->application_channel().to_app.wants_mouse_cursor_captured && !cursor_is_captured) {
                m_window.capture_mouse_cursor();
                m_ignore_mouse_move_on_next_event = true;
            } else if(!m_active_scene->application_channel().to_app.wants_mouse_cursor_captured && cursor_is_captured) {
                m_window.uncapture_mouse_cursor();
                m_ignore_mouse_move_on_next_event = true;
            }

            m_active_scene->application_channel().from_app.framebuffer_size = m_window.get_framebuf_size();
            m_active_scene->application_channel().from_app.mouse_cursor_is_captured = cursor_is_captured;


            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            float delta = std::chrono::duration<float>(curr_frame_time - last_frame_time).count();
            float frame_time = std::chrono::duration<float>(curr_frame_time - app_start_time).count();

            m_active_scene->update(delta);
            m_active_scene->render(frame_time);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            m_window.swap_buffers();
            m_window.poll_events();


            last_frame_time = curr_frame_time;
        }
    }
}
