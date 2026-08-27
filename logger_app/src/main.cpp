#include "logger/logger.h"

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

// A single log write request, passed from the input thread to the writer thread.
struct LogTask {
    std::string message;
    logger::Level level;
};

// Thread-safe producer-consumer queue connecting the input thread
// (producer) and the writer thread (consumer).
class LogQueue {
public:
    void Push(LogTask task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

    // Blocks until a task is available or Stop() has been called.
    // Returns false once the queue is empty and stopped (safe to exit).
    bool Pop(LogTask& outTask)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });

        if (queue_.empty() && stopped_) {
            return false;
        }

        outTask = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_one();
    }

private:
    std::queue<LogTask> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

// Parses "message;LEVEL" input. The level part is optional.
bool ParseInput(const std::string& input, std::string& outMessage, bool& hasLevel, logger::Level& outLevel)
{
    auto pos = input.rfind(';');

    if (pos == std::string::npos) {
        outMessage = input;
        hasLevel = false;
        return true;
    }

    outMessage = input.substr(0, pos);
    std::string levelStr = input.substr(pos + 1);

    if (levelStr == "INFO") {
        outLevel = logger::Level::Info;
    } else if (levelStr == "WARNING") {
        outLevel = logger::Level::Warning;
    } else if (levelStr == "ERROR") {
        outLevel = logger::Level::Error;
    } else {
        return false;
    }

    hasLevel = true;
    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                   << " <log_file> <default_level: INFO|WARNING|ERROR>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string levelArg = argv[2];

    logger::Level defaultLevel;
    if (levelArg == "INFO") {
        defaultLevel = logger::Level::Info;
    } else if (levelArg == "WARNING") {
        defaultLevel = logger::Level::Warning;
    } else if (levelArg == "ERROR") {
        defaultLevel = logger::Level::Error;
    } else {
        std::cerr << "Unknown importance level: " << levelArg << "\n";
        return 1;
    }

    logger::Logger log(filename, defaultLevel);

    if (!log.IsOpen()) {
        std::cerr << "Error: could not open log file '" << filename << "' for writing.\n";
        return 1;
    }

    LogQueue queue;

    // Background thread: pulls tasks off the queue and writes them to the log,
    // so the input loop below is never blocked by file I/O.
    std::thread writerThread([&queue, &log]() {
        LogTask task;
        while (queue.Pop(task)) {
            log.Log(task.message, task.level);
        }
    });

    std::cout << "Enter a message as: text;LEVEL (the level part is optional).\n";
    std::cout << "Type 'exit' to quit.\n";

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "exit") {
            break;
        }

        if (input.empty()) {
            continue;
        }

        std::string message;
        bool hasLevel = false;
        logger::Level level = log.GetDefaultLevel();

        if (!ParseInput(input, message, hasLevel, level)) {
            std::cout << "Invalid importance level, please try again.\n";
            continue;
        }

        if (!hasLevel) {
            level = log.GetDefaultLevel();
        }

        queue.Push(LogTask{message, level});
    }

    queue.Stop();
    writerThread.join();

    std::cout << "Program finished\n";
    return 0;
}