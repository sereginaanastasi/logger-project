cmake_minimum_required(VERSION 3.10)
project(logger_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(logger_lib)
add_subdirectory(logger_app)