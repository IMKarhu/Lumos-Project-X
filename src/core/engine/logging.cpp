#include "logging.h"
#include <print>

namespace Lumos
{
    //do we need a mutex at some point in time???
    //I have no fricking clue.

    //Would be nice to expand these in a way that they can take in variables like vector
    //and display vector data in a format like [x: y: z: w:]
    void ConsoleLogger::info(const std::string& message, LogLevel level)
    {
        auto loglevel = loglevelToString(level);
        std::println("\033[32m[{}]\033[0m {}",loglevel, message);
    }

    void ConsoleLogger::trace(const std::string& message, LogLevel level)
    {
        auto loglevel = loglevelToString(level);
        std::println("\033[36m[{}]\033[0m {}",loglevel, message);
    }

    void ConsoleLogger::debug(const std::string& message, LogLevel level)
    {
        auto loglevel = loglevelToString(level);
        std::println("\033[32m[{}]\033[0m {}",loglevel, message);
    }

    void ConsoleLogger::error(const std::string& message, LogLevel level)
    {
        auto loglevel = loglevelToString(level);
        std::println("\033[31m[{}]\033[0m {}",loglevel, message);
    }

    void ConsoleLogger::fatal(const std::string& message, LogLevel level)
    {
        auto loglevel = loglevelToString(level);
        std::println("\033[31m[{}]\033[0m {}",loglevel, message);
    }
} //namespace Lumos
