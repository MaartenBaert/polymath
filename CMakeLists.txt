cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

project(polymath VERSION 0.0.0)

option(POLYMATH_BUILD_BENCHMARK "Build the PolyMath benchmark executable" OFF)
option(POLYMATH_BUILD_DEMO "Build the PolyMath OpenGL demo executable (requires OpenGL and SDL2)" OFF)
option(POLYMATH_BUILD_PYTHON "Build the PolyMath python module (requires pybind11)" OFF)
option(POLYMATH_BUILD_TESTBED "Build the PolyMath testbed executable (requires Qt5)" OFF)
option(POLYMATH_BUILD_UNITTESTS "Build the PolyMath unit tests executable" ON)

option(BENCHMARK_WITH_BOOST "Compile benchmark with boost support" OFF)
option(BENCHMARK_WITH_CLIPPER "Compile benchmark with clipper support" OFF)
option(BENCHMARK_WITH_GEOS "Compile benchmark with geos support" OFF)

option(WIDEMATH_ENABLE_ASM "Allow architecture-specific assembly instructions or intrinsics for better performance." ON)

include(FeatureSummary)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
