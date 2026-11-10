cmake_minimum_required(VERSION 3.21)

project(polymath VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	endif()
endif()

# When PolyMath is used as a subproject (e.g. a git submodule), only the header-only library target (PolyMath::PolyMath)
# is defined by default.
option(POLYMATH_BUILD_BENCHMARK "Build the PolyMath benchmark executable" OFF)
option(POLYMATH_BUILD_DEMO "Build the PolyMath OpenGL demo executable (requires OpenGL and SDL2)" OFF)
option(POLYMATH_BUILD_PYTHON "Build the PolyMath python module (requires pybind11)" OFF)
option(POLYMATH_BUILD_TESTBED "Build the PolyMath testbed executable (requires Qt5)" OFF)
option(POLYMATH_BUILD_UNITTESTS "Build the PolyMath unit tests executable" ${PROJECT_IS_TOP_LEVEL})

option(BENCHMARK_WITH_BOOST "Compile benchmark with boost support" OFF)
option(BENCHMARK_WITH_CLIPPER "Compile benchmark with clipper support" OFF)
option(BENCHMARK_WITH_GEOS "Compile benchmark with geos support" OFF)

option(WIDEMATH_ENABLE_ASM "Allow architecture-specific assembly instructions or intrinsics for better performance." ON)

include(FeatureSummary)

if(POLYMATH_BUILD_UNITTESTS)
	enable_testing()
endif()

add_subdirectory(src)

if(PROJECT_IS_TOP_LEVEL)
	feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
