#pragma once
#include <array>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Diagnostics
{
enum class LogLevel
{ 
  Error,
  Warning,
  Info,
  Debug,
  Verbose
};

class ConsoleLogger
{
public:
  static void Log(const std::string& message);
};

inline void ConsoleLogger::Log(const std::string & message)
{
  std::cout << message;
}

template <typename T>
class Logger
{
public:
  Logger();
  virtual ~Logger();

  std::ostringstream& GetStream(LogLevel logLevel = LogLevel::Info);

  static LogLevel& ReportingLevel();
  static std::string ToString(LogLevel logLevel);
  static LogLevel FromString(const std::string& logLevel);

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

protected:
  std::ostringstream _stream;
};

template<typename T>
inline Logger<T>::Logger()
{  
}

template<typename T>
inline Logger<T>::~Logger()
{
  _stream << std::endl;
  T::Log(_stream.str());
}

template<typename T>
inline std::ostringstream& Logger<T>::GetStream(LogLevel logLevel)
{
  _stream << LocalTimeNow();
  _stream << " " << ToString(logLevel) << ": ";
  return _stream;
}

template <typename T>
LogLevel& Logger<T>::ReportingLevel()
{
  static LogLevel reportingLevel = LogLevel::Verbose;
  return reportingLevel;
}

template <typename T>
std::string Logger<T>::ToString(LogLevel logLevel)
{
  static std::array<std::string, 5> buffer = { "ERROR", "WARNING", "INFO", "DEBUG", "VERBOSE" };
  return buffer[static_cast<int>(logLevel)];
}

template<typename T>
inline LogLevel Logger<T>::FromString(const std::string& logLevel)
{
  switch (logLevel)
  {
    case "VERBOSE":
      return LogLevel::Verbose;
    case "DEBUG":
      return LogLevel::Debug;
    case "INFO":
      return LogLevel::Info;
    case "WARNING":
      return LogLevel::Warning;
    case "ERROR":
      return LogLevel::Error;
  }
}

inline std::string LocalTimeNow()
{
  time_t t = time(nullptr);
  struct tm tm_buff;
  char str[26];

  localtime_s(&tm_buff, &t);
  asctime_s(str, sizeof(str), &tm_buff);

  strftime(str, sizeof(str), "%F %T", &tm_buff);

  return std::string(str);
}

class LogToConsole : public Logger<ConsoleLogger> {};

#define LOG_ERROR \
  if (LogLevel::Error <= LogToConsole::ReportingLevel()) \
    LogToConsole().GetStream(LogLevel::Error)

#define LOG_WARNING \
  if (LogLevel::Warning <= LogToConsole::ReportingLevel()) \
    LogToConsole().GetStream(LogLevel::Warning)

#define LOG_INFO \
  if (LogLevel::Info <= LogToConsole::ReportingLevel()) \
    LogToConsole().GetStream(LogLevel::Info)

#define LOG_DEBUG \
  if (LogLevel::Debug <= LogToConsole::ReportingLevel()) \
    LogToConsole().GetStream(LogLevel::Debug)

#define LOG_VERBOSE \
  if (LogLevel::Verbose <= LogToConsole::ReportingLevel()) \
    LogToConsole().GetStream(LogLevel::Verbose)
}