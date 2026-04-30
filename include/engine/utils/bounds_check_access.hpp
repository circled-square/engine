#ifndef ENGINE_UTILS_BOUNDS_CHECK_ACCESS_HPP
#define ENGINE_UTILS_BOUNDS_CHECK_ACCESS_HPP

#include <cstddef> // std::size_t
#include <slogga/asserts.hpp>

namespace engine {
    inline auto& bounds_check_access(auto& vec, std::size_t index) {
        EXPECTS(index < vec.size());
        return vec[index];
    }
}

#endif // ENGINE_UTILS_BOUNDS_CHECK_ACCESS_HPP
