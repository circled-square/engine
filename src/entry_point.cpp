#include <engine/entry_point.hpp>
#include <engine/application.hpp>
#include <slogga/log.hpp>

namespace engine {
    static void terminate_handler();

    void entry_point(glm::ivec2 wnd_res, const char* wnd_name, window::creation_hints wnd_hints, std::function<rc<scene>()> get_start_scene) {
        #ifdef NDEBUG
            slogga::stdout_log.set_log_level(slogga::log_level::WARNING);
        #else
            slogga::stdout_log.set_log_level(slogga::log_level::TRACE);
        #endif

        std::set_terminate(terminate_handler);

        engine::application application(wnd_res, wnd_name, wnd_hints);

        application.set_active_scene(get_start_scene());

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
