cmake_minimum_required(VERSION 3.28.3)
project(lb_buffer C)

set(CMAKE_C_STANDARD 11)

add_executable(lb_buffer_main test/main.c)
target_include_directories(lb_buffer_main PRIVATE include)

# if GCC
#if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
#    target_compile_options(lb_buffer_main PRIVATE -Wall -Wextra -Werror -Wno-unused-parameter)
#endif()

add_executable(lb_buffer_test test/test.c)
target_include_directories(lb_buffer_test PRIVATE include)

add_executable(lb_buffer_benchmark test/benchmark.c)
target_include_directories(lb_buffer_benchmark PRIVATE include)
