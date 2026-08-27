#include "logger/logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

std::string ReadFile(const std::string& filename)
{
    std::ifstream file(filename);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Test 1: a message at or above the default level should be written to the file.
bool TestMessageAboveDefaultLevelIsWritten()
{
    const std::string filename = "test1.log";
    std::remove(filename.c_str());

    {
        logger::Logger log(filename, logger::Level::Warning);
        log.Log("important error", logger::Level::Error);
    }

    std::string content = ReadFile(filename);
    std::remove(filename.c_str());

    bool passed = content.find("important error") != std::string::npos;
    if (!passed) {
        std::cout << "  FAILED: expected 'important error' to be in the log file\n";
    }
    return passed;
}

// Test 2: a message below the default level should NOT be written to the file.
bool TestMessageBelowDefaultLevelIsFiltered()
{
    const std::string filename = "test2.log";
    std::remove(filename.c_str());

    {
        logger::Logger log(filename, logger::Level::Warning);
        log.Log("just some info", logger::Level::Info);
    }

    std::string content = ReadFile(filename);
    std::remove(filename.c_str());

    bool passed = content.find("just some info") == std::string::npos;
    if (!passed) {
        std::cout << "  FAILED: expected 'just some info' to be filtered out\n";
    }
    return passed;
}

// Test 3: SetDefaultLevel should change filtering behavior after construction.
bool TestSetDefaultLevelChangesFiltering()
{
    const std::string filename = "test3.log";
    std::remove(filename.c_str());

    {
        logger::Logger log(filename, logger::Level::Error);
        log.Log("this should be filtered", logger::Level::Info);
        log.SetDefaultLevel(logger::Level::Info);
        log.Log("this should now pass", logger::Level::Info);
    }

    std::string content = ReadFile(filename);
    std::remove(filename.c_str());

    bool passed = content.find("this should be filtered") == std::string::npos
               && content.find("this should now pass") != std::string::npos;

    if (!passed) {
        std::cout << "  FAILED: SetDefaultLevel did not change filtering as expected\n";
    }
    return passed;
}

// Test 4: the log entry should contain the correct level tag.
bool TestLogEntryContainsCorrectLevel()
{
    const std::string filename = "test4.log";
    std::remove(filename.c_str());

    {
        logger::Logger log(filename, logger::Level::Info);
        log.Log("some warning", logger::Level::Warning);
    }

    std::string content = ReadFile(filename);
    std::remove(filename.c_str());

    bool passed = content.find("[WARNING]") != std::string::npos;
    if (!passed) {
        std::cout << "  FAILED: expected '[WARNING]' tag in the log entry\n";
    }
    return passed;
}

// Test 5: IsOpen() should return true for a valid path and false for an invalid one.
bool TestIsOpenReflectsFileState()
{
    const std::string filename = "test5.log";
    std::remove(filename.c_str());

    bool validOpened;
    {
        logger::Logger log(filename, logger::Level::Info);
        validOpened = log.IsOpen();
    }
    std::remove(filename.c_str());

    logger::Logger badLog("/no_such_folder/test5.log", logger::Level::Info);
    bool invalidOpened = badLog.IsOpen();

    bool passed = validOpened && !invalidOpened;
    if (!passed) {
        std::cout << "  FAILED: IsOpen() did not correctly reflect file state\n";
    }
    return passed;
}

int main()
{
    int passedCount = 0;
    int totalCount = 0;

    #define RUN_TEST(testFunc)                          \
        do {                                             \
            totalCount++;                                \
            std::cout << "Running " #testFunc "... ";    \
            if (testFunc()) {                            \
                std::cout << "OK\n";                      \
                passedCount++;                            \
            } else {                                      \
                std::cout << "FAILED\n";                  \
            }                                              \
        } while (0)

    RUN_TEST(TestMessageAboveDefaultLevelIsWritten);
    RUN_TEST(TestMessageBelowDefaultLevelIsFiltered);
    RUN_TEST(TestSetDefaultLevelChangesFiltering);
    RUN_TEST(TestLogEntryContainsCorrectLevel);
    RUN_TEST(TestIsOpenReflectsFileState);

    std::cout << "\n" << passedCount << "/" << totalCount << " tests passed.\n";

    return (passedCount == totalCount) ? 0 : 1;
}