cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

project(polymath VERSION 0.0.0)

option(WIDEMATH_ENABLE_ASM "Allow architecture-specific assembly instructions or intrinsics for better performance." TRUE)

include(FeatureSummary)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
