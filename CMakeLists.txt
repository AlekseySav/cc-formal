cmake_minimum_required(VERSION 3.10)

project(cc)
project(tests)
include_directories("include")
set(CMAKE_CXX_STANDARD 23)

file(GLOB cc_source "src/*.cpp")
file(GLOB test_source "*/*.cpp")
list(REMOVE_ITEM test_source "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
message("${test_source}")

add_executable(cc ${cc_source})
add_executable(tests ${test_source})
target_link_libraries(tests gtest)
