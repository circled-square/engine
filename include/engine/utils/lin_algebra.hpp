#ifndef ENGINE_UTILS_LIN_ALGEBRA_HPP
#define ENGINE_UTILS_LIN_ALGEBRA_HPP
#include <glm/glm.hpp>
#include <engine/utils/api_macro.hpp>


namespace engine {
    struct mvp_matrices {
        glm::mat4 m, v, p;

        glm::mat4 compute() const { return p * v * m; }
    };

    glm::mat4 to_rotation_mat(const glm::mat4& m);
    inline glm::vec3 extract_position(const glm::mat4& m) {
        //NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        return glm::vec3(m[3]);
        //NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    }
}

#endif // ENGINE_UTILS_LIN_ALGEBRA_HPP