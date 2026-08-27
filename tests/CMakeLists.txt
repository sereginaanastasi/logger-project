add_executable(logger_tests test_logger.cpp)

# Тесты используют ту же библиотеку, что и основное приложение.
target_link_libraries(logger_tests PRIVATE logger_shared)