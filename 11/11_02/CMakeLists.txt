cmake_minimum_required(VERSION 3.20)
project(call_dispatch_bench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(benchmark REQUIRED)

add_executable(call_dispatch_bench main.cpp)
target_link_libraries(call_dispatch_bench PRIVATE benchmark::benchmark)

if (MSVC)
    target_compile_options(call_dispatch_bench PRIVATE /O2)
else()
    target_compile_options(call_dispatch_bench PRIVATE -O3 -DNDEBUG)
endif()