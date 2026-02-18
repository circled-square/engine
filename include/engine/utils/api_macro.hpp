#ifndef ENGINE_UTILS_API_MACRO_HPP
#define ENGINE_UTILS_API_MACRO_HPP


#if defined(_MSC_VER) || defined(_WIN32) // Microsoft
#    define ENGINE_EXPORT __declspec(dllexport)
#    define ENGINE_IMPORT __declspec(dllimport)
#else // GCC, or hopefully something GCC compatible
#    define ENGINE_EXPORT [[gnu::visibility("default")]]
#    define ENGINE_IMPORT [[gnu::visibility("default")]]
#endif

#if defined(ENGINE_BEING_COMPILED)
#    define ENGINE_API ENGINE_EXPORT
#else
#    define ENGINE_API ENGINE_IMPORT
#endif


#endif // ENGINE_UTILS_API_MACRO_HPP
