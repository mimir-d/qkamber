#pragma once

namespace details
{
	class LoggerImpl
	{
	public:
		class LogPrefix
		{
		public:
			void incr();
			void decr();
			std::string format(const std::string& message);

		private:
			int m_tabindex;
		};
		static LogPrefix prefix;

		enum Level
		{
			Debug,
			Info,
			Warning,
			Error
		};

		static void Log(Level lev, const char* message, ...);
	};
	
	class RaiiLogger
	{
	public:
		RaiiLogger(const char* function, const char* filename, int lineno);
		~RaiiLogger();

	private:
		std::string m_function;
	};
}

#define log(level, msg, ...) details::LoggerImpl::Log(level, msg, __VA_ARGS__) 

#define log_debug(msg, ...) do { log(details::LoggerImpl::Debug, (msg), __VA_ARGS__); } while(0)
#define log_info(msg, ...)  do { log(details::LoggerImpl::Info, (msg), __VA_ARGS__); } while(0)
#define log_warn(msg, ...)  do { log(details::LoggerImpl::Warning, (msg), __VA_ARGS__); } while(0)
#define log_error(msg, ...) do { log(details::LoggerImpl::Error, (msg), __VA_ARGS__); } while(0)

#define __CONCAT(x, y) x ## y
#define __VARNAME(prefix) __CONCAT(prefix, __LINE__)
#define flog() \
	auto __VARNAME(__raiilogger_) = \
	details::RaiiLogger(__FUNCTION__, (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__), __LINE__)
