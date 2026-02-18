#ifndef GAL_API_MACRO_HPP
#define GAL_API_MACRO_HPP


// (handled by cmake) define GAL_AS_SHARED_LIB in all translation units if gal is meant to be included (and exported) by a shared lib; also define it in translation units which make use of gal through the shared lib
#if defined(GAL_AS_SHARED_LIB)
#    if defined(_MSC_VER) || defined(_WIN32) // Microsoft
#        define GAL_EXPORT __declspec(dllexport)
#        define GAL_IMPORT __declspec(dllimport)
#    else // GCC, or hopefully something GCC compatible
#        define GAL_EXPORT [[gnu::visibility("default")]]
#        define GAL_IMPORT [[gnu::visibility("default")]]
#    endif
#else
#    define GAL_EXPORT
#    define GAL_IMPORT
#endif

// (handled by cmake) define GAL_BEING_COMPILED in all of gal's translation units, but not in translation units wanting to import gal
#if defined(GAL_BEING_COMPILED)
#    define GAL_API GAL_EXPORT
#else
#    define GAL_API GAL_IMPORT
#endif


#endif // GAL_API_MACRO_HPP
