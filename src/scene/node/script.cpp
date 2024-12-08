#include <engine/scene/node/script.hpp>


namespace engine {
    script::script(rc<const stateless_script> sl_script, std::any state) : m_script(std::move(sl_script)), m_state(std::move(state)) {}

    script::script(rc<const stateless_script> sl_script) : m_script(std::move(sl_script)), m_state(m_script->construct()) {}

    script engine::script::copy_with_default_state() { return script(m_script, m_script->construct()); }

    const std::any& script::get_state() const { return m_state; }

    void script::process(node& n, application_channel_t& app_chan) { m_script->process(n, m_state, app_chan); }

    void script::attach(node& n) { if(m_script->attach) m_script->attach(n, m_state); }
}
