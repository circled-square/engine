#include <engine/application.hpp>
#include <slogga/log.hpp>
#include <glm/glm.hpp>

#include <engine/entry_point.hpp>
#include <engine/scene/yaml_loader.hpp>

#ifndef ENGINE_BEING_COMPILED
#error ENGINE_BEING_COMPILED macro should be defined for all engine translation units!
#endif

namespace engine {
    static void terminate_handler();

    void entry_point(std::optional<slogga::log_level> log_level_override, std::string project_file_path, glm::ivec2 wnd_res, const std::string& wnd_name, window::hints wnd_hints) {
        project_info_t project_info = load_yaml_project_file(project_file_path.c_str());

        slogga::log_level log_level = log_level_override.value_or(
            project_info.log_level.value_or(
                #ifdef NDEBUG
                    slogga::log_level::WARN
                #else
                    slogga::log_level::TRACE
                #endif
            )
        );
        slogga::stdout_log.set_log_level(log_level);


        std::set_terminate(terminate_handler);

        engine::application application(
            project_info.resolution.value_or({1280, 720}), //NOLINT(cppcoreguidelines-avoid-magic-numbers) // this is just a fairly sensible default resolution; the user is expected to set it themselves anyway.
            project_info.window_name,
            window::hints { .maximised = project_info.maximised }
        );

        
        application.set_start_scene(get_rm().load_mut<scene>(project_info.start_scene_path));

        application.run();
    }

    static void terminate_handler() {
        slogga::stdout_log("terminate called!");
        try {
            std::rethrow_exception(std::current_exception());
        } catch(const std::exception& e) {
            slogga::stdout_log.fatal("uncaught exception: {}", e.what());
        } catch(...) {
            slogga::stdout_log.fatal("uncaught exception with unknown type");
        }
    }
}
