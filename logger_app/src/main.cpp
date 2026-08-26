#include "logger/logger.h"

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <sstream>

// One "task" to be written to the log — this is what we put in the queue.
struct LogTask {
    std::string message;
    logger::Level level;
};

// Thread-safe wrapper around the message queue.
class LogQueue {
public:
    // Add a task to the queue (called from the main/input thread).
    void Push(LogTask task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            // lock_guard automatically locks mutex_ on creation and
            // unlocks it when the scope ends. This protects queue_
            // from being accessed by two threads at the same time.
            queue_.push(std::move(task));
        }
        cv_.notify_one(); // wake up the writer thread if it's sleeping
    }

    // Take a task from the queue (called from the writer thread).
    // If the queue is empty, the thread sleeps here instead of busy-waiting.
    // Returns false if a stop signal was received and the queue is empty
    // (meaning it's time to shut down).
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

    // Signal the writer thread that no more tasks will arrive —
    // needed for a clean shutdown.
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

// Parses user input.
// Expected format: "message text;LEVEL" — the level part is optional.
// Example: "Server started;INFO" or just "Server started" (uses the default level).
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
        return false; // unknown level — treat input as invalid
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
    LogQueue queue;

    // Start the writer thread. It runs in the background for the
    // whole lifetime of the program.
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
            break; // input stream ended (e.g. Ctrl+D)
        }

        if (input == "exit") {
            break;
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

    // Tell the writer thread no more messages will come, and wait for it to finish.
    queue.Stop();
    writerThread.join();

    std::cout << "Program finished.\n";
    return 0;
}