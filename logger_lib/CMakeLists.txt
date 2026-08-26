# Собираем библиотеку в двух вариантах: статическая и динамическая.
# Используем один и тот же исходник (logger.cpp) для обеих версий.

add_library(logger_static STATIC src/logger.cpp)
add_library(logger_shared SHARED src/logger.cpp)

# Указываем, где искать заголовочные файлы (logger.h) — 
# и для самой библиотеки, и для всех, кто её подключит через target_link_libraries.
target_include_directories(logger_static PUBLIC include)
target_include_directories(logger_shared PUBLIC include)

# Задаём стандарт C++17, как требуется в задании.
target_compile_features(logger_static PUBLIC cxx_std_17)
target_compile_features(logger_shared PUBLIC cxx_std_17)

# Чтобы оба варианта не назывались одинаково на диске (liblogger_static.a / liblogger_shared.so),
# но при желании можно унифицировать имя вывода:
set_target_properties(logger_static PROPERTIES OUTPUT_NAME logger)
set_target_properties(logger_shared PROPERTIES OUTPUT_NAME logger)