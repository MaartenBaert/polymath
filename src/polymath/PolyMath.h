#pragma once

#include "Common.h"

#include "Polygon.h"
#include "PolygonPoint.h"
#include "SweepEngine.h"
#include "Vertex.h"
#include "Visualization.h"
#include "WindingEngine.h"

namespace PolyMath {

template<typename T, typename WindingWeight>
Polygon<T> PolygonSimplify_NonZero(const Polygon<T, WindingWeight> &polygon) {
	SweepEngine<T, WindingEngine_NonZero<WindingWeight>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename WindingWeight>
Polygon<T> PolygonSimplify_EvenOdd(const Polygon<T, WindingWeight> &polygon) {
	SweepEngine<T, WindingEngine_EvenOdd<WindingWeight>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename WindingWeight>
Polygon<T> PolygonSimplify_Positive(const Polygon<T, WindingWeight> &polygon) {
	SweepEngine<T, WindingEngine_Positive<WindingWeight>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename WindingWeight>
Polygon<T> PolygonSimplify_Negative(const Polygon<T, WindingWeight> &polygon) {
	SweepEngine<T, WindingEngine_Negative<WindingWeight>> engine(polygon);
	engine.Process();
	return engine.Result();
}

}
