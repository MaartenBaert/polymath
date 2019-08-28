#pragma once

#include "Common.h"

#include "Polygon.h"
#include "PolygonPoint.h"
#include "SweepEngine.h"
#include "Vertex.h"
#include "Visualization.h"
#include "WindingEngine.h"

namespace PolyMath {

template<typename T, typename W>
Polygon<T> PolygonSimplify_NonZero(const Polygon<T, W> &polygon) {
	SweepEngine<T, WindingEngine_NonZero<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W>
Polygon<T> PolygonSimplify_EvenOdd(const Polygon<T, W> &polygon) {
	SweepEngine<T, WindingEngine_EvenOdd<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W>
Polygon<T> PolygonSimplify_Positive(const Polygon<T, W> &polygon) {
	SweepEngine<T, WindingEngine_Positive<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W>
Polygon<T> PolygonSimplify_Negative(const Polygon<T, W> &polygon) {
	SweepEngine<T, WindingEngine_Negative<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

}
