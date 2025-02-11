#include <engine/scene/node/script.hpp>

#include <engine/scene/node/narrow_phase_collision.hpp>


namespace engine {
    script::script(rc<const stateless_script> sl_script, std::any state) : m_script(std::move(sl_script)), m_state(std::move(state)) {
        EXPECTS(m_state.has_value());
    }

    script::script(rc<const stateless_script> sl_script, const noderef& n) : m_script(std::move(sl_script)), m_state(m_script->construct(n)) {
        EXPECTS(m_state.has_value());
    }
    script script::from_state(rc<const stateless_script> sl_script, std::any state) {
        return script(std::move(sl_script), std::move(state));
    }

    const std::any& script::get_state() const { return m_state; }

    const rc<const stateless_script> script::get_underlying_stateless_script() const { return m_script; }

    void script::process(const noderef& n, application_channel_t& app_chan) { m_script->process(n, m_state, app_chan); }

    void script::react_to_collision(const node& self, collision_result res, const node& event_src, const node& other) {
        EXPECTS(m_script->react_to_collision != nullptr);
        m_script->react_to_collision(self, this->m_state, res, event_src, other);
    }
}
