#ifndef ENGINE_SCENE_NODE_SCRIPT_HPP
#define ENGINE_SCENE_NODE_SCRIPT_HPP

#include <any>
#include <vector>
#include <engine/resources_manager/rc.hpp>
#include <engine/utils/api_macro.hpp>

namespace engine {
    class node_data;
    class node;
    class application_channel_t;
    class collision_result;

    struct script_vtable {
        using construct_fn_t = std::any (const node&);
        using process_fn_t = void (const node&, std::any&, application_channel_t&);
        // NOTE: react_to_collision takes "const node_data&"s because it should not move or delete any nodes, since it gets called after subscribing "node*"s to the bp collision detector
        using react_to_collision_fn_t = void (const node_data&, std::any&, collision_result, const node_data& event_src, const node_data& other);

        construct_fn_t* construct = [](const node&) { return std::any(std::monostate()); };
        process_fn_t* process = [](const node&, std::any&, application_channel_t&) {};
        // NOTE: react_to_collision takes "const node_data&"s because it should not move or delete any nodes, since it gets called after subscribing "node*"s to the bp collision detector
        std::optional<react_to_collision_fn_t*> react_to_collision = std::nullopt;

    };

    struct stateless_script {
        script_vtable vtable = script_vtable{};
        std::string name = std::string();
        // this pointer is necessary to make sure the dynamic library is not unloaded while the object's fn ptrs still point to it
        rc<const dylib::library> dynlib_ref = nullptr;

        ENGINE_API static std::vector<std::pair<const char*, stateless_script>> from(rc<const dylib::library> dynlib);
        ENGINE_API static stateless_script from(rc<const dylib::library> dynlib, const char* name);
    };

    class script {
        stateless_script m_script;
        std::any m_state;

        script(stateless_script sl_script, std::any state);//used by from_state()
    public:
        script() = delete;
        script(const script& o) = default;
        script(script&& o) = default;
        ENGINE_API script(stateless_script sl_script, const node& n);
        ENGINE_API static script from_state(stateless_script sl_script, std::any state);

        script& operator=(script&& o) = default;

        ENGINE_API void set_state(std::any s);
        ENGINE_API const std::any& get_state() const;
        ENGINE_API const stateless_script& get_underlying_stateless_script() const;

        ENGINE_API void process(const node& n, application_channel_t& app_chan);
        ENGINE_API void react_to_collision(const node_data& self, collision_result res, const node_data& event_src, const node_data& other);
    };
}

#endif //ENGINE_SCENE_NODE_SCRIPT_HPP
