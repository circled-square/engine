#ifndef ENGINE_UTILS_CONSTANTS_HPP
#define ENGINE_UTILS_CONSTANTS_HPP

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {
    inline namespace constants {
        [[maybe_unused]]
        constexpr glm::vec3 x_axis(1,0,0), y_axis(0,1,0), z_axis(0,0,1);

        [[maybe_unused]]
        constexpr float pi = glm::pi<float>();
    }
}

#endif //ENGINE_UTILS_CONSTANTS_HPP
