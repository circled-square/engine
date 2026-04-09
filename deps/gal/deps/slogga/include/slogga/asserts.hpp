#ifndef SLOGGA_ASSERTS_HPP
#define SLOGGA_ASSERTS_HPP

#include <format>
#include <stdexcept>
#include <source_location>

// Attribute assume allows the compiler to optimize assuming the expression will always evaluate to true.
// This should always be the case provided the code was properly tested.
#ifdef __has_cpp_attribute
	#if __has_cpp_attribute(assume)
		#define SLOGGA_ASSUME(x) [[assume(x)]]
	#elif __has_cpp_attribute(gnu::assume)
		#define SLOGGA_ASSUME(x) [[gnu::assume(x)]]
	#else
		#define SLOGGA_ASSUME(x) //nothing
	#endif
#else
	#define SLOGGA_ASSUME(x) //nothing
#endif

// Macros are used to avoid evaluation of the condition in release builds
#ifdef NDEBUG
    // Empty implementation for release builds

    #define EXPECTS(x) SLOGGA_ASSUME(x)
    #define ENSURES(x) SLOGGA_ASSUME(x)
    #define ASSERTS(x) SLOGGA_ASSUME(x)

    #define EXPECTS_WITH_MSG(x, msg) SLOGGA_ASSUME(x)
    #define ENSURES_WITH_MSG(x, msg) SLOGGA_ASSUME(x)
    #define ASSERTS_WITH_MSG(x, msg) SLOGGA_ASSUME(x)
#else
    // Actual implementation for debug builds
    #define EXPECTS(x) (slogga::detail::assertion(x, #x, "precondition check"))
    #define ENSURES(x) (slogga::detail::assertion(x, #x, "postcondition check"))
    #define ASSERTS(x) (slogga::detail::assertion(x, #x, "assertion"))

    #define EXPECTS_WITH_MSG(x, msg) (slogga::detail::assertion(x, #x, "precondition check", msg))
    #define ENSURES_WITH_MSG(x, msg) (slogga::detail::assertion(x, #x, "postcondition check", msg))
    #define ASSERTS_WITH_MSG(x, msg) (slogga::detail::assertion(x, #x, "assertion", msg))
#endif

// UNIMPLEMENTED is not empty even for release builds: the input that causes it to fail is
// actually valid and the user does not deserve to get undefined behaviour (that's what
// assume does) rather than a crash/exception: instead they'd rather always pay the small
// price of an assertion. Besides, ideally, any UNIMPLEMENTED check is eventually removed,
// whereas other assertions are meant to stay indefinitely.
#define UNIMPLEMENTED(x) (slogga::detail::assertion(x, #x, "this feature is not implemented; assertion"))
#define UNIMPLEMENTED_WITH_MSG(x, msg) (slogga::detail::assertion(x, #x, "this feature is not implemented; assertion", msg))


namespace slogga {
    class assertion_failed_exception : public std::exception {
        const char* m_cond_str;
        const char* m_assertion_type;
        std::source_location m_sl;
        std::string m_custom_msg;
        mutable std::string m_what_string_cache = std::string();
    public:
        assertion_failed_exception(const char* cond_str, const char* assertion_type, std::string msg, std::source_location sl) :
            m_cond_str(cond_str), m_assertion_type(assertion_type), m_custom_msg(std::move(msg)), m_sl(std::move(sl)) {}

        virtual const char* what() const noexcept override {
            if(m_what_string_cache.empty()) {
                if(m_custom_msg.empty()) {
                    m_what_string_cache = std::format("{} failed at function '{}' at {}:{}:{}: '{}' evaluated to false",
                        m_assertion_type, m_sl.function_name(), m_sl.file_name(), m_sl.line(), m_sl.column(), m_cond_str);
                } else {
                    m_what_string_cache = std::format("{} failed: {} ('{}' evaluated to false)",
                        m_assertion_type, m_custom_msg, m_cond_str);
                }
            }
            return m_what_string_cache.c_str();
        }
    };

    namespace detail {
        inline void assertion(bool cond, const char* cond_str, const char* assertion_type, std::string msg = "", std::source_location sl = std::source_location::current()) {
    	    if(!cond) {
                throw assertion_failed_exception(cond_str, assertion_type, std::move(msg), std::move(sl));
    	    }
    	}
    }
}

#endif // SLOGGA_ASSERTS_HPP
