#include "logger/logger.h"

#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace logger {

// Здесь мы определяем, что скрывается за Impl (см. logger.h).
// Это "настоящие" приватные данные класса Logger.
class Logger::Impl {
public:
    std::ofstream file;       // поток для записи в файл журнала
    Level defaultLevel;       // текущий уровень важности по умолчанию

    Impl(const std::string& filename, Level level)
        : defaultLevel(level)
    {
        // std::ios::app — открываем файл в режиме "дописывания" (append),
        // а не перезаписи, чтобы старые записи в журнале не стирались
        // при каждом запуске программы.
        file.open(filename, std::ios::app);
    }
};

// Конструктор Logger просто создаёт объект Impl и запоминает указатель на него.
Logger::Logger(const std::string& filename, Level defaultLevel)
    : pImpl(new Impl(filename, defaultLevel))
{
}

// Деструктор — освобождаем память, выделенную под Impl.
// std::ofstream сам закроет файл в своём деструкторе, нам не нужно делать это вручную.
Logger::~Logger()
{
    delete pImpl;
}

// Вспомогательная функция (не часть публичного API, поэтому в anonymous namespace —
// она видна только внутри этого .cpp файла, и не "торчит" наружу).
namespace {

    // Переводит Level в текстовое имя для записи в файл.
    std::string LevelToString(Level level)
    {
        switch (level) {
            case Level::Info:    return "INFO";
            case Level::Warning: return "WARNING";
            case Level::Error:   return "ERROR";
        }
        return "UNKNOWN"; // на случай, если появится новый уровень, а мы забудем его добавить сюда
    }

    // Возвращает текущее время в виде строки "YYYY-MM-DD HH:MM:SS".
    std::string CurrentTimeString()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        std::tm tm_buf{};
        localtime_r(&now_c, &tm_buf); // потокобезопасная версия localtime (важно для Части 2!)

        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

} // anonymous namespace

bool Logger::Log(const std::string& message, Level level)
{
    // Фильтрация: если уровень сообщения ниже уровня по умолчанию — не пишем.
    if (level < pImpl->defaultLevel) {
        return true; // это не ошибка, просто сообщение отфильтровано
    }

    if (!pImpl->file.is_open()) {
        return false; // не получилось открыть файл — сообщаем об ошибке через bool
    }

    pImpl->file << "[" << CurrentTimeString() << "] "
                << "[" << LevelToString(level) << "] "
                << message << std::endl;

    return true;
}

void Logger::SetDefaultLevel(Level level)
{
    pImpl->defaultLevel = level;
}

Level Logger::GetDefaultLevel() const
{
    return pImpl->defaultLevel;
}

} // namespace logger