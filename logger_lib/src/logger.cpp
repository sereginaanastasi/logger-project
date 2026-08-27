#include "logger/logger.h"

#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace logger {

class Logger::Impl {
public:
    std::ofstream file;
    Level defaultLevel;
    mutable std::mutex mutex;

    Impl(const std::string& filename, Level level)
        : defaultLevel(level)
    {
        file.open(filename, std::ios::app);
    }
};

Logger::Logger(const std::string& filename, Level defaultLevel)
    : pImpl(new Impl(filename, defaultLevel))
{
}

Logger::~Logger()
{
    delete pImpl;
}

namespace {

    std::string LevelToString(Level level)
    {
        switch (level) {
            case Level::Info:    return "INFO";
            case Level::Warning: return "WARNING";
            case Level::Error:   return "ERROR";
        }
        return "UNKNOWN";
    }

    std::string CurrentTimeString()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        std::tm tm_buf{};
        localtime_r(&now_c, &tm_buf); // thread-safe variant of localtime

        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

} // anonymous namespace

bool Logger::Log(const std::string& message, Level level)
{
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    if (level < pImpl->defaultLevel) {
        return true; // filtered out, not an error
    }

    if (!pImpl->file.is_open()) {
        return false;
    }

    pImpl->file << "[" << CurrentTimeString() << "] "
                << "[" << LevelToString(level) << "] "
                << message << std::endl;

    return true;
}

void Logger::SetDefaultLevel(Level level)
{
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->defaultLevel = level;
}

Level Logger::GetDefaultLevel() const
{
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->defaultLevel;
}

bool Logger::IsOpen() const
{
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->file.is_open();
}

} // namespace logger