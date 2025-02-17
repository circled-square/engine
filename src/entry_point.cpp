#include <engine/entry_point.hpp>
#include <engine/application.hpp>
#include <slogga/log.hpp>

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}


namespace engine {
    static void terminate_handler();

    void entry_point(glm::ivec2 wnd_res, const std::string& wnd_name, window::hints wnd_hints, std::function<rc<scene>()> get_start_scene) {
        #ifdef NDEBUG
            slogga::stdout_log.set_log_level(slogga::log_level::WARN);
        #else
            slogga::stdout_log.set_log_level(slogga::log_level::TRACE);
        #endif


        lua_State* L = luaL_newstate();
        luaL_dostring(L, "a = 5 + 5");



        std::set_terminate(terminate_handler);

        engine::application application(wnd_res, wnd_name, wnd_hints);

        application.set_start_scene(get_start_scene());

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
