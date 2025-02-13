#include <engine/scene/node/camera.hpp>

namespace engine {
    camera::camera(const camera& o) : m_view_mat(o.m_view_mat) {}

    camera::camera(const glm::mat4& view_mat) : m_view_mat(view_mat) { }

    const glm::mat4& camera::get_view_mat() const { return m_view_mat; }

    void camera::set_view_mat(const glm::mat4& view_mat) { m_view_mat = view_mat; }
}
