set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(benchmark_sources
	benchmark/3rdparty/clipper.cpp
	benchmark/3rdparty/clipper.hpp
	benchmark/BoostWrapper.cpp
	benchmark/BoostWrapper.h
	benchmark/ClipperWrapper.cpp
	benchmark/ClipperWrapper.h
	benchmark/GeosWrapper.cpp
	benchmark/GeosWrapper.h
	benchmark/Main.cpp
	benchmark/PolyMathWrapper.cpp
	benchmark/PolyMathWrapper.h
	benchmark/Wrappers.h
)

set(polymath_sources
	polymath/Common.h
	polymath/NumericalEngine.h
	polymath/Polygon.h
	polymath/PolygonPoint.h
	polymath/PolyMath.h
	polymath/SweepEngine.h
	polymath/Vertex.h
	polymath/Visualization.h
	polymath/WindingEngine.h
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

qt5_wrap_cpp(testbed_moc
	testbed/MainWindow.h
	testbed/Visualizer.h
)

add_executable(polymath-benchmark
	${benchmark_sources}
	${polymath_sources}
	${testgenerators_sources}
	${widemath_sources}
)
target_link_libraries(polymath-benchmark PRIVATE
	-lgeos
)
target_compile_definitions(polymath-benchmark PRIVATE
	${widemath_defines}
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

add_executable(polymath-unittests
	${polymath_sources}
	${testgenerators_sources}
	${unittests_sources}
	${widemath_sources}
)
target_compile_definitions(polymath-unittests PRIVATE
	${widemath_defines}
)