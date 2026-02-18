#ifndef ENGINE_SCENE_NODE_CAMERA_HPP
#define ENGINE_SCENE_NODE_CAMERA_HPP

#include <glm/glm.hpp>
#include <engine/utils/api_macro.hpp>

namespace engine {
    class camera {
        glm::mat4 m_view_mat;
    public:
        ENGINE_API camera(const camera& o);
        ENGINE_API camera(const glm::mat4& view_matrix = glm::mat4(1));
        ENGINE_API const glm::mat4& get_view_mat() const;
        ENGINE_API void set_view_mat(const glm::mat4& view_mat);
    };
}


#endif // ENGINE_SCENE_NODE_CAMERA_HPP
