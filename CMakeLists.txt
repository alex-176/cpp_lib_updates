cmake_minimum_required(VERSION 3.11)
project(api_updates)

set(CMAKE_CXX_STANDARD 17)

add_library(api_updates src/api.cpp)
target_include_directories(api_updates PUBLIC include)

add_executable(test_api tests/main.cpp)
target_link_libraries(test_api api_updates)
