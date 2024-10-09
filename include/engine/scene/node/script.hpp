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
        script(const script& o) : m_script(o.m_script), m_state(o.m_state) {}
        script(script&& o) : m_script(std::move(o.m_script)), m_state(std::move(o.m_state)) {}
        script(rc<const stateless_script> sl_script, std::any state) : m_script(std::move(sl_script)), m_state(std::move(state)) {}
        script(rc<const stateless_script> sl_script) : m_script(std::move(sl_script)), m_state(m_script->construct()) {}
        script copy_with_default_state() { return script(m_script, m_script->construct()); }

        script& operator=(script&& o) {
            m_script = std::move(o.m_script);
            m_state = std::move(o.m_state);
            return *this;
        }

        const std::any& get_state() const { return m_state; }

        void process(node& n, application_channel_t& app_chan) { m_script->process(n, m_state, app_chan); }
        void attach(node& n) { if(m_script->attach) m_script->attach(n, m_state); }
    };
}

#endif //ENGINE_SCENE_NODE_SCRIPT_HPP
