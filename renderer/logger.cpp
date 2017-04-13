
#include "stdafx.h"
using namespace std;

namespace details
{
	void LoggerImpl::LogPrefix::incr()
	{
		m_tabindex ++;
	}

	void LoggerImpl::LogPrefix::decr()
	{
		m_tabindex --;
	}

	string LoggerImpl::LogPrefix::format(const string& message)
	{
		std::ostringstream ostr;
		for (int i = 0; i < m_tabindex; i++)
			ostr << "    ";
		ostr << message << std::ends;
		return ostr.str();
	}

	LoggerImpl::LogPrefix LoggerImpl::prefix;

	void LoggerImpl::Log(LoggerImpl::Level lev, const char* message, ...)
	{
		va_list args;
		va_start(args, message);
			
		std::ostringstream ostr;
		ostr << prefix.format(message).c_str() << std::endl << std::ends;
		vprintf(ostr.str().c_str(), args);
			
		va_end(args);
	}

	RaiiLogger::RaiiLogger(const char* function, const char* filename, int lineno) :
		m_function(function)
	{
		LoggerImpl::Log(LoggerImpl::Debug, "Entered %s in %s:%d", function, filename, lineno);	
		LoggerImpl::prefix.incr();
	}

	RaiiLogger::~RaiiLogger()
	{
		LoggerImpl::prefix.decr();
		LoggerImpl::Log(LoggerImpl::Debug, "Exited %s", m_function.c_str());
	}
}