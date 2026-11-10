if(POLYMATH_BUILD_BENCHMARK)
	if(BENCHMARK_WITH_BOOST)
		find_package(Boost CONFIG REQUIRED)
	endif()
	if(BENCHMARK_WITH_GEOS)
		find_package(GEOS 3.12 CONFIG REQUIRED)
	endif()
endif()

if(POLYMATH_BUILD_DEMO)
	find_package(OpenGL REQUIRED)
	find_package(SDL2 REQUIRED)
endif()

if(POLYMATH_BUILD_PYTHON)
	set(PYBIND11_FINDPYTHON ON)
	find_package(pybind11 REQUIRED)
endif()

if(POLYMATH_BUILD_TESTBED)
	find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
endif()

set(benchmark_sources
	benchmark/Main.cpp
	benchmark/PolyMathWrapper.cpp
	benchmark/PolyMathWrapper.h
	benchmark/Wrappers.h
)
if(BENCHMARK_WITH_BOOST)
	list(APPEND benchmark_sources
		benchmark/BoostWrapper.cpp
		benchmark/BoostWrapper.h
	)
	list(APPEND benchmark_libs
		Boost::headers
	)
endif()
if(BENCHMARK_WITH_CLIPPER)
	list(APPEND benchmark_sources
		benchmark/3rdparty/clipper1/clipper1.cpp
		benchmark/3rdparty/clipper1/clipper1.hpp
		benchmark/3rdparty/clipper2/clipper2.h
		benchmark/3rdparty/clipper2/clipper2.core.h
		benchmark/3rdparty/clipper2/clipper2.engine.cpp
		benchmark/3rdparty/clipper2/clipper2.engine.h
		benchmark/3rdparty/clipper2/clipper2.export.h
		benchmark/3rdparty/clipper2/clipper2.minkowski.h
		benchmark/3rdparty/clipper2/clipper2.offset.cpp
		benchmark/3rdparty/clipper2/clipper2.offset.h
		benchmark/3rdparty/clipper2/clipper2.rectclip.cpp
		benchmark/3rdparty/clipper2/clipper2.rectclip.h
		benchmark/Clipper1Wrapper.cpp
		benchmark/Clipper1Wrapper.h
		benchmark/Clipper2Wrapper.cpp
		benchmark/Clipper2Wrapper.h
	)
endif()
if(BENCHMARK_WITH_GEOS)
	list(APPEND benchmark_sources
		benchmark/GeosWrapper.cpp
		benchmark/GeosWrapper.h
	)
	list(APPEND benchmark_libs
		GEOS::geos
	)
endif()

set(demo_sources
	demo/Main.cpp
)

set(polymath_sources
	polymath/Common.h
	polymath/NumericalEngine.h
	polymath/OutputPolicy.h
	polymath/Polygon.h
	polymath/PolygonPoint.h
	polymath/PolyMath.h
	polymath/SweepEngine.h
	polymath/SweepTree.h
	polymath/Vertex.h
	polymath/Visualization.h
	polymath/WindingPolicy.h
)

set(python_sources
	python/Polygon.cpp
)

set(testbed_sources
	testbed/Main.cpp
	testbed/MainWindow.cpp
	testbed/MainWindow.h
	testbed/Qt.h
	testbed/Visualizer.cpp
	testbed/Visualizer.h
)

set(testgenerators_sources
	testgenerators/TestGenerators.cpp
	testgenerators/TestGenerators.h
	testgenerators/TypeConverter.h
)

set(unittests_sources
	unittests/3rdparty/catch.hpp
	unittests/GoldenData.inc
	unittests/Main.cpp
	unittests/TestCommon.h
	unittests/TestGolden.cpp
	unittests/TestOutput.cpp
	unittests/TestWideMath.cpp
)

set(widemath_sources
	widemath/Add.h
	widemath/Common.h
	widemath/Compare.h
	widemath/Divide.h
	widemath/Misc.h
	widemath/Multiply.h
	widemath/Negate.h
	widemath/Shift.h
	widemath/Sqrt.h
	widemath/Subtract.h
	widemath/WideMath.h
)

# Header-only library target. Users include "polymath/PolyMath.h". The header lists above are only used for reference.
add_library(polymath-headers INTERFACE)
add_library(PolyMath::PolyMath ALIAS polymath-headers)
target_include_directories(polymath-headers INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_features(polymath-headers INTERFACE
	cxx_std_17
)
target_compile_definitions(polymath-headers INTERFACE
	WIDEMATH_USE_ASM=$<BOOL:${WIDEMATH_ENABLE_ASM}>
)

# Internal library for test polygon generators, shared by several executables.
if(POLYMATH_BUILD_BENCHMARK OR POLYMATH_BUILD_TESTBED OR POLYMATH_BUILD_UNITTESTS)
	add_library(polymath-testgenerators STATIC
		${testgenerators_sources}
	)
	target_link_libraries(polymath-testgenerators PUBLIC
		PolyMath::PolyMath
	)
endif()

if(POLYMATH_BUILD_BENCHMARK)
	add_executable(polymath-benchmark
		${benchmark_sources}
	)
	target_link_libraries(polymath-benchmark PRIVATE
		PolyMath::PolyMath
		polymath-testgenerators
		${benchmark_libs}
	)
	target_compile_definitions(polymath-benchmark PRIVATE
		BENCHMARK_WITH_BOOST=$<BOOL:${BENCHMARK_WITH_BOOST}>
		BENCHMARK_WITH_CLIPPER=$<BOOL:${BENCHMARK_WITH_CLIPPER}>
		BENCHMARK_WITH_GEOS=$<BOOL:${BENCHMARK_WITH_GEOS}>
	)
endif()

if(POLYMATH_BUILD_DEMO)
	add_executable(polymath-demo
		${demo_sources}
	)
	target_link_libraries(polymath-demo PRIVATE
		PolyMath::PolyMath
		OpenGL::GL
		SDL2::SDL2
	)
endif()

if(POLYMATH_BUILD_PYTHON)
	pybind11_add_module(polymath
		${python_sources}
	)
	target_link_libraries(polymath PRIVATE
		PolyMath::PolyMath
	)
endif()

if(POLYMATH_BUILD_TESTBED)
	add_executable(polymath-testbed
		${testbed_sources}
	)
	set_target_properties(polymath-testbed PROPERTIES
		AUTOMOC ON
	)
	target_link_libraries(polymath-testbed PRIVATE
		PolyMath::PolyMath
		polymath-testgenerators
		Qt5::Core
		Qt5::Gui
		Qt5::Widgets
	)
endif()

if(POLYMATH_BUILD_UNITTESTS)
	add_executable(polymath-unittests
		${unittests_sources}
	)
	target_link_libraries(polymath-unittests PRIVATE
		PolyMath::PolyMath
		polymath-testgenerators
	)
	add_test(NAME polymath-unittests COMMAND polymath-unittests)
endif()
