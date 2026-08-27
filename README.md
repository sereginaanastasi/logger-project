# Logger Library

A simple C++17 logging library with configurable severity levels, plus a multithreaded console application that demonstrates its usage.

## Project structure

- `logger_lib/` — the logging library (Part 1)
  - `include/` — public header (`logger.h`)
  - `src/` — implementation (`logger.cpp`)
- `logger_app/` — console application using the library (Part 2)
  - `src/` — `main.cpp`
- `tests/` — unit tests for the library
- `CMakeLists.txt` — root build configuration

## Requirements

- C++17 compiler (tested with GCC 13)
- CMake 3.10+
- Ubuntu/Debian (or any modern Linux distribution)
- No external dependencies — STL only

## Building

```bash
mkdir build && cd build
cmake ..
make
```

This builds three targets:
- `liblogger.a` / `liblogger.so` — the logging library (static and shared)
- `logger_app` — the console application
- `logger_tests` — the unit test suite

## Running the application

```bash
./logger_app/logger_app <log_file> <default_level>
```

`<default_level>` must be one of: `INFO`, `WARNING`, `ERROR`.

Example:

```bash
./logger_app/logger_app app.log WARNING
```

Then enter messages in the format `text;LEVEL` (the level is optional and falls back to the current default). For example, typing `Server started;INFO`, then `Something went wrong;ERROR`, then `Connection lost` (no level), then `exit`.

Messages below the current default level are silently filtered out and not written to the log file. Message handling runs on a dedicated background thread using a thread-safe queue, so the input prompt is never blocked by file I/O.

## Running the tests

```bash
./tests/logger_tests
```

## Library API

```cpp
#include "logger/logger.h"

logger::Logger log("app.log", logger::Level::Warning);

log.Log("Something happened", logger::Level::Error);  // written
log.Log("Just some info", logger::Level::Info);        // filtered out (below default)

log.SetDefaultLevel(logger::Level::Info);               // change the default level
log.Log("Now this passes too", logger::Level::Info);

if (!log.IsOpen()) {
    // handle log file open failure
}
```

Each log entry is written in the format: `[YYYY-MM-DD HH:MM:SS] [LEVEL] message text`

The library is thread-safe: `Log()`, `SetDefaultLevel()`, and `GetDefaultLevel()` can be called concurrently from multiple threads.