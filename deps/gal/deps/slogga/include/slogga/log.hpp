#ifndef SLOGGA_LOG_HPP
#define SLOGGA_LOG_HPP

#include <format>
#include <variant>
#include <fstream>

// (handled by cmake) define SLOGGA_AS_SHARED_LIB in all translation units if slogga is meant to be included (and exported) by a shared lib; also define it in translation units which make use of slogga through the shared lib
#if defined(SLOGGA_AS_SHARED_LIB)
#    if defined(_MSC_VER) || defined(_WIN32) // Microsoft
#        define SLOGGA_EXPORT __declspec(dllexport)
#        define SLOGGA_IMPORT __declspec(dllimport)
#    else // GCC, or hopefully something GCC compatible
#        define SLOGGA_EXPORT [[gnu::visibility("default")]]
#        define SLOGGA_IMPORT [[gnu::visibility("default")]]
#    endif
#else
#    define SLOGGA_EXPORT
#    define SLOGGA_IMPORT
#endif

// (handled by cmake) define SLOGGA_BEING_COMPILED in all of slogga's translation units, but not in translation units wanting to import slogga
#if defined(SLOGGA_BEING_COMPILED)
#    define SLOGGA_API SLOGGA_EXPORT
#else
#    define SLOGGA_API SLOGGA_IMPORT
#endif

namespace slogga {

    /*
     * //for example to create a log in append mode:
     * slogga::log log(std::ofstream("log/file/path", std::ios::app));
     *
     * this is obviously not thread safe.
     */
    enum class log_level : int {
        OFF = -1,
        FATAL = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4,
        TRACE = 5,
    };
    #ifdef NDEBUG
        constexpr log_level default_log_level = log_level::WARN;
    #else
        constexpr log_level default_log_level = log_level::DEBUG;
    #endif

    class log {
        std::variant<std::ofstream, std::FILE*> m_stream;
        mutable std::string m_timestamp;
        mutable std::string m_last_line;
        mutable std::size_t m_repeated_line_count;
        log_level m_log_level;

    public:
        log() = delete;
        log(log&&) = delete;
        log(const log&) = delete;
        log& operator=(log&&) = delete;
        log& operator=(const log&) = delete;

        SLOGGA_API log(std::variant<std::ofstream, std::FILE*> os, log_level level, bool timestamp = false);
        SLOGGA_API ~log();

        SLOGGA_API void set_log_level(log_level l);

        SLOGGA_API void end_current_line(); // prints the repetition count (if >= 1) and a newline, ensures the next line gets printed regardless of equality with the previous

        SLOGGA_API void operator()(log_level l, std::string_view fmt, std::format_args args);

        SLOGGA_API bool would_print(log_level l) const;

        //shorthand for .trace()
        inline void operator()(std::string_view s, auto...args) { this->trace(s, args...); }

        inline void trace(std::string_view s, auto...args) { operator()(log_level::TRACE, s, std::make_format_args(args...)); }
        inline void debug(std::string_view s, auto...args) { operator()(log_level::DEBUG, s, std::make_format_args(args...)); }
        inline void info (std::string_view s, auto...args) { operator()(log_level::INFO,  s, std::make_format_args(args...)); }
        inline void warn (std::string_view s, auto...args) { operator()(log_level::WARN,  s, std::make_format_args(args...)); }
        inline void error(std::string_view s, auto...args) { operator()(log_level::ERROR, s, std::make_format_args(args...)); }
        inline void fatal(std::string_view s, auto...args) { operator()(log_level::FATAL, s, std::make_format_args(args...)); }
    };

    SLOGGA_API extern log stdout_log; //NOLINT(cppcoreguidelines-avoid-non-const-global-variables) // the guideline is to avoid the use of non-const globals but explicitly does not forbid them, using std::cout as an example of a valid usage. This object is essentially the same thing.
}


#endif // SLOGGA_LOG_HPP
