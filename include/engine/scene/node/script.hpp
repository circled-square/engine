#ifndef ENGINE_SCENE_NODE_SCRIPT_HPP
#define ENGINE_SCENE_NODE_SCRIPT_HPP

#include <any>
#include <engine/resources_manager/rc.hpp>

namespace engine {
    class node;
    class noderef;
    class application_channel_t;
    class collision_result;

    struct stateless_script {
        std::any (*construct)(const noderef&) = [](const noderef&) { return std::any(std::monostate()); };
        void (*process)(const noderef&, std::any&, application_channel_t&) = [](const noderef&, std::any&, application_channel_t&) {};
        // NOTE: this method takes "const node&"s because it should not move or delete any nodes, since it gets called after subscribing "node*"s to the bp collision detector
        void (*react_to_collision)(const node&, std::any&, collision_result, const node& event_src, const node& other) = nullptr;
    };

    class script {
        rc<const stateless_script> m_script;
        std::any m_state;

        script(rc<const stateless_script> sl_script, std::any state);//used by from_state()
    public:
        script() = delete;
        script(const script& o) = default;
        script(script&& o) = default;
        script(rc<const stateless_script> sl_script, const noderef& n);
        static script from_state(rc<const stateless_script> sl_script, std::any state);

        script& operator=(script&& o) = default;

        const std::any& get_state() const;
        void set_state(std::any s) { m_state = std::move(s); }
        const rc<const stateless_script> get_underlying_stateless_script() const;

        void process(const noderef& n, application_channel_t& app_chan);
        void react_to_collision(const node& self, collision_result res, const node& event_src, const node& other);
    };
}

#endif //ENGINE_SCENE_NODE_SCRIPT_HPP
