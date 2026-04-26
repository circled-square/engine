#include "engine/scene/node.hpp"
#include <engine/scene/node/script.hpp>

#include <engine/scene/node/narrow_phase_collision.hpp>
#include <dylib.hpp>


namespace engine {
    script::script(stateless_script sl_script, node& n, const std::any& params) : m_script(std::move(sl_script)), m_state(m_script.vtable.construct(n, params)) {
        EXPECTS(m_state.has_value());
    }

    const std::any& script::get_state() const { return m_state; }

    const stateless_script& script::get_underlying_stateless_script() const { return m_script; }

    void script::process(node& n, application_channel_t& app_chan) { m_script.vtable.process(n, m_state, app_chan); }

    void script::react_to_collision(const node& self, collision_result res, const node& event_src, const node& other) {
        EXPECTS(m_script.vtable.react_to_collision != std::nullopt);
        (*m_script.vtable.react_to_collision)(self, this->m_state, res, event_src, other);
    }

    std::vector<std::pair<const char*, stateless_script> > stateless_script::from(rc<const dylib::library> dynlib) {
        const std::size_t imported_plugins_size = dynlib->get_variable<std::size_t>("exported_plugins_size");
        const std::pair<const char*, script_vtable> (&imported_plugins)[] = dynlib->get_variable<const std::pair<const char*, script_vtable>[]>("exported_plugins"); // NOLINT(cppcoreguidelines-avoid-c-arrays)

        std::vector<std::pair<const char*, stateless_script>> ret;
        ret.reserve(imported_plugins_size);

        for(int i = 0; i < imported_plugins_size; i++) {
            stateless_script s {
                .vtable = imported_plugins[i].second,
                .name = imported_plugins[i].first,
                .dynlib_ref = nullable_rc<const dylib::library>(dynlib),
            };
            ret.push_back(std::pair(imported_plugins[i].first, std::move(s)));
        }
        return std::move(ret);
    }

    stateless_script stateless_script::from(rc<const dylib::library> dynlib, const char *name) {
        const std::size_t imported_plugins_size = dynlib->get_variable<std::size_t>("exported_plugins_size");
        const std::pair<const char*, script_vtable> (&imported_plugins)[] = dynlib->get_variable<const std::pair<const char*, script_vtable>[]>("exported_plugins"); // NOLINT(cppcoreguidelines-avoid-c-arrays)

        for(int i = 0; i < imported_plugins_size; i++) {
            if(std::strcmp(imported_plugins[i].first, name) == 0) {

                return stateless_script {
                    .vtable = imported_plugins[i].second,
                    .name = imported_plugins[i].first,
                    .dynlib_ref = dynlib,
                };
            }
        }

        throw std::runtime_error(std::format("script with name {} not found in dynamic library", name));
    }

}
