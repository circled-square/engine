#ifndef ENGINE_UTILS_FORMAT_VEC_HPP
#define ENGINE_UTILS_FORMAT_VEC_HPP
#include <sstream>

template<typename T, int N, glm::qualifier Q>
struct std::formatter<glm::vec<N, T, Q>, char>
{
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<class FmtContext>
    FmtContext::iterator format(glm::vec<N,T,Q> v, FmtContext& ctx) const {
        std::ostringstream out;
        //prevent u8/i8 being printed as characters
        if constexpr (std::same_as<T, std::int8_t> || std::same_as<T, std::uint8_t>) {
            out << "(" << (int)v.x << ", " << (int)v.y << ", " << (int)v.z << ")";
        } else {
            out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        }

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

#endif // ENGINE_UTILS_FORMAT_VEC_HPP
