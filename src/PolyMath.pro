QT += core gui widgets
TARGET = polymath
TEMPLATE = app

INCLUDEPATH += misc polymath testbed wrappers
LIBS += -lgeos

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
	testbed/Visualizer.cpp \
	wrappers/BoostWrapper.cpp \
	wrappers/ClipperWrapper.cpp \
	wrappers/GeosWrapper.cpp \
	wrappers/PolyMathWrapper.cpp \
    testbed/TestGenerators.cpp

HEADERS  += \
	misc/clipper.hpp \
	testbed/MainWindow.h \
	testbed/Qt.h \
	testbed/Visualizer.h \
	polymath/Common.h \
	polymath/Polygon.h \
	polymath/PolyMath.h \
	polymath/SweepEngine.h \
	polymath/Vertex.h \
	polymath/Visualization.h \
	polymath/WindingEngine.h \
	wrappers/BoostWrapper.h \
	wrappers/ClipperWrapper.h \
	wrappers/GeosWrapper.h \
	wrappers/PolyMathWrapper.h \
	wrappers/Wrappers.h \
    polymath/PolygonPoint.h \
    testbed/TestGenerators.h
