#ifndef ENGINE_SCENE_NODE_SCRIPT_HPP
#define ENGINE_SCENE_NODE_SCRIPT_HPP

#include <memory>
#include <any>
#include "../../rc.hpp"

namespace engine {
    class node;
    class application_channel_t;

    struct stateless_script {
        std::any (*construct)() = []() { return std::any(); };
        void (*attach)(node&, std::any&) = nullptr;
        void (*process)(node&, std::any&, application_channel_t&) = [](node&, std::any&, application_channel_t&) {};
    };

    class script {
        rc<const stateless_script> m_script;
        std::any m_state;
    public:
        script() = delete;
        script(const script& o) = default;
        script(script&& o) = default;
        script(rc<const stateless_script> sl_script, std::any state);
        script(rc<const stateless_script> sl_script);
        script copy_with_default_state();

        script& operator=(script&& o) = default;

        const std::any& get_state() const;

        void process(node& n, application_channel_t& app_chan);
        void attach(node& n);
    };
}

#endif //ENGINE_SCENE_NODE_SCRIPT_HPP
