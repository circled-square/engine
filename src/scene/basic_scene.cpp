#include <engine/scene/basic_scene.hpp>
#include <engine/application.hpp>

namespace engine {
    basic_scene::basic_scene() : m_application_channel() {}

    basic_scene::basic_scene(basic_scene &&o) : m_application_channel(std::move(o.m_application_channel)) {}

    application_channel_t &basic_scene::application_channel() { return m_application_channel; }
}
