#ifndef ENGINE_SCENE_RENDERER_CAMERA_HPP
#define ENGINE_SCENE_RENDERER_CAMERA_HPP

#include <glm/glm.hpp>

namespace engine {
    class camera {
        glm::mat4 m_view_mat;
    public:
        camera(glm::mat4 view_matrix = glm::mat4(1));
        glm::mat4 get_view_mat() const;
        void set_view_mat(glm::mat4 view_mat);
    };
}


#endif // ENGINE_SCENE_RENDERER_CAMERA_HPP
