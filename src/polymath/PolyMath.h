#pragma once

#include "Common.h"

#include "Polygon.h"
#include "PolygonPoint.h"
#include "SweepEngine.h"
#include "Vertex.h"
#include "Visualization.h"
#include "WindingEngine.h"

namespace PolyMath {

template<typename T, typename WindingNumber>
Polygon<T> PolygonSimplify_NonZero(const Polygon<T, WindingNumber> &polygon) {
	SweepEngine<T, WindingEngine_NonZero<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename WindingNumber>
Polygon<T> PolygonSimplify_EvenOdd(const Polygon<T, WindingNumber> &polygon) {
	SweepEngine<T, WindingEngine_EvenOdd<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename WindingNumber>
Polygon<T> PolygonSimplify_Positive(const Polygon<T, WindingNumber> &polygon) {
	SweepEngine<T, WindingEngine_Positive<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename WindingNumber>
Polygon<T> PolygonSimplify_Negative(const Polygon<T, WindingNumber> &polygon) {
	SweepEngine<T, WindingEngine_Negative<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

}
