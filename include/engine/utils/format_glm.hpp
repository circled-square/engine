#ifndef ENGINE_UTILS_FORMAT_VEC_HPP
#define ENGINE_UTILS_FORMAT_VEC_HPP
#include "glm/detail/qualifier.hpp"
#include <sstream>
#include <format>
#include <glm/glm.hpp>

//formatter for vectors
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
            out << "(";
            for(int i = 0; i < N-1; i++) {
                out << (std::int16_t)v[i] << ", ";
            }
            out << (std::int16_t)v[N-1] << ")";
        } else {
            out << "(";
            for(int i = 0; i < N-1; i++) {
                out << v[i] << ", ";
            }
            out << v[N-1] << ")";
        }

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};


//formatter for matrices
template<typename T, int N, int M, glm::qualifier Q>
struct std::formatter<glm::mat<N, M, T, Q>, char>
{
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<class FmtContext>
    FmtContext::iterator format(glm::mat<N,M,T,Q> m, FmtContext& ctx) const {
        std::ostringstream out;

        out << "[";
        for(int r = 0; r < M; r++) {
            out << (r==0 ? "(" : "), (");
            for(int c = 0; c < N; c++) {
                out << m[c][r];
                if (c < N-1)
                    out << ", ";
            }
        }
        out << ")]";

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};


#endif // ENGINE_UTILS_FORMAT_VEC_HPP
