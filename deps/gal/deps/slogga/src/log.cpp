#include "../include/slogga/log.hpp"
#include <sstream>
#include <cstring>

namespace slogga {
    //match the extern declaration in the header
    log stdout_log(stdout, default_log_level); //NOLINT(cppcoreguidelines-avoid-non-const-global-variables) // the guideline is to avoid the use of non-const globals but explicitly does not forbid them, using std::cout as an example of a valid usage. This object is essentially the same thing.

    static void write_to_stream(std::variant<std::ofstream, std::FILE*>& stream, const char* str) {
        if(std::holds_alternative<std::ofstream>(stream)) {
            std::ofstream& s = std::get<std::ofstream>(stream);
            s << str << std::flush;
        } else if (std::holds_alternative<std::FILE*>(stream)){
            FILE* f = std::get<std::FILE*>(stream);
            std::fwrite(str, sizeof(char), std::strlen(str), f);
            std::fflush(f);
        }
    }

    static std::string get_timestamp_string() {
        std::time_t t = std::time(nullptr);
        std::tm& p = *std::localtime(&t);
        // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
        return std::format(" {}/{}/{} {}:{}:{}", p.tm_mday, p.tm_mon+1, p.tm_year%100, p.tm_hour, p.tm_min, p.tm_sec);
        // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
    }

    static constexpr const char* log_level_name(log_level l) {
        switch(l) {
            case log_level::OFF: return "OFF";
            case log_level::FATAL: return "FATAL";
            case log_level::ERROR: return "ERROR";
            case log_level::WARN: return "WARNING";
            case log_level::INFO: return "INFO";
            case log_level::DEBUG: return "DEBUG";
            case log_level::TRACE: return "TRACE";
        }
        return "(NO SUCH LOG LEVEL)";
    }


    log::log(std::variant<std::ofstream, std::FILE*> stream, log_level l, bool timestamp)
        : m_stream(std::move(stream)),
          m_timestamp(timestamp ? get_timestamp_string() : std::string()),
          m_repeated_line_count(0),
          m_log_level(l)
    {
        info("log level set to {}", log_level_name(l));
    }

    void log::set_log_level(log_level l) {
        m_log_level = l;
        info("log level set to {}", log_level_name(l));
    }

    void log::end_current_line() {
        std::stringstream ss;

        //print the count of how many times the last line was printed and terminate it
        if (m_repeated_line_count > 1) {
            ss << " (repeated x" << m_repeated_line_count << ")" << std::endl;
        } else if (m_repeated_line_count > 0) {
            ss << std::endl;
        }
        m_repeated_line_count = 0;

        write_to_stream(m_stream, ss.str().c_str());
    }

    void log::operator()(log_level l, std::string_view fmt, std::format_args args) {
        if (would_print(l)) {
            //only perform formatting if necessary
            std::string line = std::vformat(fmt, args);

            if (line == m_last_line && m_repeated_line_count != 0) {
                //repeated the last line
                m_repeated_line_count++;
            } else {
                //a new, different line is being printed
                end_current_line();

                std::stringstream ss;
                //print the new line without the \n
                ss << "[" << log_level_name(l) << m_timestamp << "] " << line;

                write_to_stream(m_stream, ss.str().c_str());

                m_repeated_line_count = 1;
                m_last_line = std::move(line);
            }
        }
    }

    bool log::would_print(log_level l) const {
        return l <= m_log_level;
    }

    // TODO: this is not called for stdout_log since it is a global
    log::~log() {
        end_current_line();
    }
}
