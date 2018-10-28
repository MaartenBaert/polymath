QT += core gui widgets
TARGET = polymath
TEMPLATE = app

INCLUDEPATH += misc polymath testbed wrappers
LIBS += -lgeos -lquadmath

QMAKE_CXXFLAGS += -std=c++11 -Wconversion -Wsign-conversion -Wfloat-conversion
QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
#QMAKE_CXXFLAGS_RELEASE += -ffast-math

pg {
	QMAKE_CFLAGS += -pg
	QMAKE_CXXFLAGS += -pg
	QMAKE_LFLAGS += -pg
}

SOURCES += \
	misc/clipper.cpp \
	testbed/Main.cpp \
	testbed/MainWindow.cpp \
	testbed/TestGenerators.cpp \
	testbed/Visualizer.cpp \
	wrappers/BoostWrapper.cpp \
	wrappers/ClipperWrapper.cpp \
	wrappers/GeosWrapper.cpp \
	wrappers/PolyMathWrapper.cpp

HEADERS  += \
	misc/clipper.hpp \
	polymath/Common.h \
	polymath/NumericalEngine.h \
	polymath/PolyMath.h \
	polymath/Polygon.h \
	polymath/PolygonPoint.h \
	polymath/SweepEngine.h \
	polymath/Vertex.h \
	polymath/Visualization.h \
	polymath/WindingEngine.h \
	testbed/MainWindow.h \
	testbed/Qt.h \
	testbed/TestGenerators.h \
	testbed/Visualizer.h \
	wrappers/BoostWrapper.h \
	wrappers/ClipperWrapper.h \
	wrappers/GeosWrapper.h \
	wrappers/PolyMathWrapper.h \
	wrappers/Wrappers.h
