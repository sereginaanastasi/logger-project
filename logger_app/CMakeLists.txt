add_executable(logger_app src/main.cpp)

# Линкуем приложение с динамической версией библиотеки.
# (Если нужна демонстрация статической линковки — можно сделать
# отдельную цель logger_app_static, слинкованную с logger_static)
target_link_libraries(logger_app PRIVATE logger_shared)

# Нужно для std::thread — без этого на Linux может не слинковаться.
find_package(Threads REQUIRED)
target_link_libraries(logger_app PRIVATE Threads::Threads)