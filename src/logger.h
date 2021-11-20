#pragma once

#include "misc.h"

constexpr char* DEFAULT_LOG = "qkamber.log";

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

class LogFormatter
{
public:
    template <typename... Args>
    std::string format(
        LogLevel level, const std::string& filename, int lineno,
        const std::string& fmt, const Args&... args
    );

    void incr_prefix(int delta);

private:
    std::string m_log_format;
    int m_tabindex;
};

class Logger
{
public:
    Logger();
    ~Logger() = default;

    static Logger& get();

    void set_output_file(const std::string& filename);
    void incr_prefix(int delta = 1);

    template <typename... Args>
    void log(LogLevel level, const char* filename, int lineno, const std::string& message, const Args&... args);

private:
    LogFormatter m_formatter;
    std::ofstream m_file;
};

namespace detail
{
	class RaiiLogger
	{
	public:
        template <typename... Args>
        RaiiLogger(
            const char* function,
            const char* filename, int lineno,
            const std::string& msg, const Args&... args
        );
		RaiiLogger(const char* function, const char* filename, int lineno);
		~RaiiLogger();

	private:
        std::string m_filename;
        int m_lineno;
		std::string m_function;
	};
}

#define log_any(level, msg, ...) \
    do { \
        Logger::get().log(level, __FILE_SHORT__, __LINE__, (msg), ## __VA_ARGS__); \
    } while (0)

#define log_debug(msg, ...) log_any(LogLevel::Debug,   (msg), ## __VA_ARGS__)
#define log_info(msg, ...)  log_any(LogLevel::Info,    (msg), ## __VA_ARGS__)
#define log_warn(msg, ...)  log_any(LogLevel::Warning, (msg), ## __VA_ARGS__)
#define log_error(msg, ...) log_any(LogLevel::Error,   (msg), ## __VA_ARGS__)

#ifdef _DEBUG
#   define dlog log_debug
#   define flog(...)                                                              \
        auto __LINE_VARNAME(__raiilogger_) =                                      \
        detail::RaiiLogger(__FUNCTION__, __FILE_SHORT__, __LINE__, ## __VA_ARGS__)
#else
#   define dlog(...)
#   define flog(...)
#endif

///////////////////////////////////////////////////////////////////////////////
// LogFormatter
///////////////////////////////////////////////////////////////////////////////
template <typename... Args>
inline std::string LogFormatter::format(
    LogLevel level, const std::string& filename, int lineno,
    const std::string& fmt, const Args&... args
) {
    using namespace std;
    using namespace std::chrono;
    ostringstream ostr;

    // write the current time since app start
    auto now = app_clock::now();
    auto time_parts = duration_parts<hours, minutes, seconds, milliseconds>(now.time_since_epoch());
    ostr << setw(2) << setfill('0') << get<0>(time_parts).count() << ":";
    ostr << setw(2) << setfill('0') << get<1>(time_parts).count() << ":";
    ostr << setw(2) << setfill('0') << get<2>(time_parts).count() << ".";
    ostr << setw(3) << setfill('0') << get<3>(time_parts).count() << " ";

    // write the log level
    switch (level)
    {
        case LogLevel::Debug:   ostr << "DEBUG: "; break;
        case LogLevel::Info:    ostr << "INFO : "; break;
        case LogLevel::Warning: ostr << "WARN : "; break;
        case LogLevel::Error:   ostr << "ERROR: "; break;
    }

    // write filename:lineno
    ostr << setw(30) << setfill(' ') << left << (filename + ":" + std::to_string(lineno)) << "| ";

    // write the actual message
    for (int i = 0; i < m_tabindex; i++)
        ostr << "    ";
    ostr << print_fmt(fmt, args...);

    return ostr.str();
}

inline void LogFormatter::incr_prefix(int delta)
{
    m_tabindex += delta;
}

///////////////////////////////////////////////////////////////////////////////
// Logger impl
///////////////////////////////////////////////////////////////////////////////
inline Logger::Logger()
{
    set_output_file(DEFAULT_LOG);
}

inline Logger& Logger::get()
{
    static Logger log;
    return log;
}

inline void Logger::set_output_file(const std::string& filename)
{
    if (m_file.is_open())
        m_file.close();

    m_file.open(filename, std::ios::out);
    if (!m_file)
        throw std::runtime_error("unable to open log file");
    m_file.rdbuf()->pubsetbuf(0, 0);
}


inline void Logger::incr_prefix(int delta)
{
    m_formatter.incr_prefix(delta);
}

template <typename... Args>
inline void Logger::log(LogLevel level, const char* filename, int lineno, const std::string& message, const Args&... args)
{
    std::string out = m_formatter.format(level, filename, lineno, message, args...);
    std::cout << out << std::endl;
    if (m_file)
        m_file << out <<std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// RaiiLogger impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <typename... Args>
    inline RaiiLogger::RaiiLogger(
        const char* function,
        const char* filename, int lineno,
        const std::string& fmt, const Args&... args
    ) : m_function(function), m_filename(filename), m_lineno(lineno)
    {
        using namespace std::string_literals;

        auto& logger = Logger::get();
        if (fmt.length() > 0)
            logger.log(LogLevel::Debug, filename, lineno, "--> %s, "s + fmt, function, args...);
        else
            logger.log(LogLevel::Debug, filename, lineno, "--> %s", function);
        logger.incr_prefix();
    }

    inline RaiiLogger::RaiiLogger(const char* function, const char* filename, int lineno) :
        RaiiLogger(function, filename, lineno, "")
    {}

    inline RaiiLogger::~RaiiLogger()
    {
        auto& logger = Logger::get();
        logger.incr_prefix(-1);
        logger.log(LogLevel::Debug, m_filename.c_str(), m_lineno, "<-- %s", m_function.c_str());
    }
}