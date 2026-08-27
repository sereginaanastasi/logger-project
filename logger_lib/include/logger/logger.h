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

    // Writes a message to the log if its level is high enough.
    // Thread-safe: can be called concurrently from multiple threads.
    bool Log(const std::string& message, Level level);

    // Changes the default importance level after construction.
    // Thread-safe.
    void SetDefaultLevel(Level level);

    // Returns the current default importance level.
    // Thread-safe.
    Level GetDefaultLevel() const;

        // Returns true if the log file was successfully opened and is ready to accept writes.
    bool IsOpen() const;

private:
    class Impl;
    Impl* pImpl;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

} // namespace logger