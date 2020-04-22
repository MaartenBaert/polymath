set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(POLYMATH_BUILD_DEMO)
	find_package(OpenGL REQUIRED)
	find_package(SDL2 REQUIRED)
endif()

if(POLYMATH_BUILD_PYTHON)
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
	list(append benchmark_sources
		benchmark/BoostWrapper.cpp
		benchmark/BoostWrapper.h
	)
endif()
if(BENCHMARK_WITH_CLIPPER)
	list(append benchmark_sources
		benchmark/3rdparty/clipper.cpp
		benchmark/3rdparty/clipper.hpp
		benchmark/ClipperWrapper.cpp
		benchmark/ClipperWrapper.h
	)
endif()
if(BENCHMARK_WITH_GEOS)
	list(append benchmark_sources
		benchmark/GeosWrapper.cpp
		benchmark/GeosWrapper.h
	)
	list(append benchmark_libs
		-lgeos
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
	unittests/Main.cpp
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

set(widemath_defines
	-DWIDEMATH_USE_ASM=$<BOOL:${WIDEMATH_ENABLE_ASM}>
)

if(POLYMATH_BUILD_BENCHMARK)
	add_executable(polymath-benchmark
		${benchmark_sources}
		${polymath_sources}
		${testgenerators_sources}
		${widemath_sources}
	)
	target_link_libraries(polymath-benchmark PRIVATE
		${benchmark_libs}
	)
	target_compile_definitions(polymath-benchmark PRIVATE
		${widemath_defines}
		-DBENCHMARK_WITH_BOOST=$<BOOL:${BENCHMARK_WITH_BOOST}>
		-DBENCHMARK_WITH_CLIPPER=$<BOOL:${BENCHMARK_WITH_CLIPPER}>
		-DBENCHMARK_WITH_GEOS=$<BOOL:${BENCHMARK_WITH_GEOS}>
	)
endif()

if(POLYMATH_BUILD_DEMO)
	add_executable(polymath-demo
		${demo_sources}
		${polymath_sources}
		${widemath_sources}
	)
	target_link_libraries(polymath-demo PRIVATE
		OpenGL::GL
		SDL2::SDL2
	)
	target_compile_definitions(polymath-demo PRIVATE
		${widemath_defines}
	)
endif()

if(POLYMATH_BUILD_PYTHON)
	pybind11_add_module(polymath
		${polymath_sources}
		${python_sources}
	)
endif()

if(POLYMATH_BUILD_TESTBED)
	qt5_wrap_cpp(testbed_moc
		testbed/MainWindow.h
		testbed/Visualizer.h
	)
	add_executable(polymath-testbed
		${polymath_sources}
		${testbed_sources}
		${testbed_moc}
		${testgenerators_sources}
		${widemath_sources}
	)
	target_link_libraries(polymath-testbed PRIVATE
		Qt5::Core
		Qt5::Gui
		Qt5::Widgets
	)
	target_compile_definitions(polymath-testbed PRIVATE
		${widemath_defines}
	)
endif()

if(POLYMATH_BUILD_UNITTESTS)
	add_executable(polymath-unittests
		${polymath_sources}
		${testgenerators_sources}
		${unittests_sources}
		${widemath_sources}
	)
	target_compile_definitions(polymath-unittests PRIVATE
		${widemath_defines}
	)
endif()
