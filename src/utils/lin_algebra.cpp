#include <engine/utils/lin_algebra.hpp>

namespace engine {
    using namespace glm;
    mat4 to_rotation_mat(const mat4& m) {
        mat4 mat = m;
        //NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        mat[3] = vec4(0);
        //NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        return mat;
    }
}