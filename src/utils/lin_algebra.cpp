#include <engine/utils/lin_algebra.hpp>

namespace engine {
    using namespace glm;
    mat4 to_rotation_mat(const mat4& m) {
        mat4 mat = m;
        mat[3] = vec4(0);
        return mat;
    }
    vec3 extract_position(const mat4& m) {
        return vec3(m[3]);
    }
}