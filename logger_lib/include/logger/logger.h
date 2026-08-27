#pragma once

#include <string>

namespace logger {

enum class Level {
    Info = 0,
    Warning = 1,
    Error = 2
};

// Allows comparing importance levels with <, <=, >= etc.
inline bool operator<(Level lhs, Level rhs)
{
    return static_cast<int>(lhs) < static_cast<int>(rhs);
}

class Logger {
public:
    Logger(const std::string& filename, Level defaultLevel);
    ~Logger();

    // Writes a message to the log if its level is >= the current default level.
    // Thread-safe.
    bool Log(const std::string& message, Level level);

    // Thread-safe.
    void SetDefaultLevel(Level level);
    Level GetDefaultLevel() const;

    // Returns false if the log file could not be opened.
    bool IsOpen() const;

private:
    class Impl;
    Impl* pImpl;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

} // namespace logger