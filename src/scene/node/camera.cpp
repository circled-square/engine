#include <engine/scene/node/camera.hpp>

namespace engine {
    camera::camera(glm::mat4 view_mat) : m_view_mat(view_mat) { }

    glm::mat4 camera::get_view_mat() const { return m_view_mat; }

    void camera::set_view_mat(glm::mat4 view_mat) { m_view_mat = view_mat; }
}
