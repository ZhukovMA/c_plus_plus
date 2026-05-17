cmake_minimum_required(VERSION 3.26)

project(json_number_benchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_Declare(
    google_benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.5
)

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG v1.1.0
)

FetchContent_MakeAvailable(nlohmann_json google_benchmark)

FetchContent_GetProperties(rapidjson)

if (NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()

add_executable(json_number_benchmarks 13_5.cpp)

target_link_libraries(
    json_number_benchmarks
    PRIVATE
    nlohmann_json::nlohmann_json
    benchmark::benchmark
)

target_include_directories(
    json_number_benchmarks
    PRIVATE
    ${rapidjson_SOURCE_DIR}/include
)

if (MSVC)
    target_compile_options(json_number_benchmarks PRIVATE /O2)
else()
    target_compile_options(json_number_benchmarks PRIVATE -O3 -march=native)
endif()