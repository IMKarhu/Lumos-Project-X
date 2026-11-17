#pragma once
#include <string>

namespace Lumos
{
#define LS_LOG(logger, level, message) (logger).log(message, level)
#define LS_LOG_TRACE(logger, message) LS_LOG(logger, LogLevel::Trace, message)
#define LS_LOG_INFO(logger, message) LS_LOG(logger, LogLevel::Info, message)
#define LS_LOG_DEBUG(logger, message) LS_LOG(logger, LogLevel::Debug, message)
#define LS_LOG_ERROR(logger, message) LS_LOG(logger, LogLevel::Error, message)
#define LS_LOG_FATAL(logger, message) LS_LOG(logger, LogLevel::Fatal, message)

    enum class LogLevel
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    /*
     * Basically trying out CRTP instead of the more traditional way of doing this with virtual base class etc..
     * With CRTP we get to avoid doing vtable lookup everytime we call Logger::log()
     */
    template <typename T>
    class Logger
    {
    public:
        void log(const std::string& message, LogLevel level) {
            switch(level)
            {
                case LogLevel::Trace:
                    static_cast<T*>(this)->trace(message, level);
                    break;
                case LogLevel::Info:
                    static_cast<T*>(this)->info(message, level);
                    break;
                case LogLevel::Debug:
                    static_cast<T*>(this)->debug(message, level);
                    break;
                case LogLevel::Error:
                    static_cast<T*>(this)->error(message, level);
                    break;
                case LogLevel::Fatal:
                    static_cast<T*>(this)->fatal(message, level);
                    break;
                default:
                    static_cast<T*>(this)->info(message, level);
                    break;
            }
        }
    protected:
        // std::mutex m_logmutex; MAYBE NEED THIS AT SOME POINT IN TIME?
        std::string loglevelToString(LogLevel level) {
            switch(level)
            {
                case LogLevel::Trace:
                    return "TRACE";
                case LogLevel::Info:
                    return "INFO";
                case LogLevel::Debug:
                    return "DEBUG";
                case LogLevel::Error:
                    return "ERROR";
                case LogLevel::Fatal:
                    return "FATAL";
                default:
                    return "INFO";
            }
        }
    private:
    };

    class ConsoleLogger : public Logger<ConsoleLogger>
    {
    public:
        void info(const std::string& message, LogLevel level);
        void trace(const std::string& message, LogLevel level);
        void debug(const std::string& message, LogLevel level);
        void error(const std::string& message, LogLevel level);
        void fatal(const std::string& message, LogLevel level);
    private:
    };
} // namespace Lumos
