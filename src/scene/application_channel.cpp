#include <engine/scene/application_channel.hpp>

namespace engine {
    application_channel_t::application_channel_t(to_app_t to_app, from_app_t from_app) : m_to_app(std::move(to_app)), m_from_app(std::move(from_app)) {}
}
