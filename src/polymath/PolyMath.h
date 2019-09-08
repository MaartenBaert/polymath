#pragma once

#include "Common.h"

#include "OutputPolicy.h"
#include "Polygon.h"
#include "PolygonPoint.h"
#include "SweepEngine.h"
#include "Vertex.h"
#include "Visualization.h"
#include "WindingPolicy.h"

namespace PolyMath {

template<typename T, typename W = default_winding_t>
Polygon<T> PolygonSimplify_NonZero(const Polygon<T, W> &polygon) {
	SweepEngine<T, OutputPolicy_Simple<T>, WindingPolicy_NonZero<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W = default_winding_t>
Polygon<T> PolygonSimplify_EvenOdd(const Polygon<T, W> &polygon) {
	SweepEngine<T, OutputPolicy_Simple<T>, WindingPolicy_EvenOdd<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W = default_winding_t>
Polygon<T> PolygonSimplify_Positive(const Polygon<T, W> &polygon) {
	SweepEngine<T, OutputPolicy_Simple<T>, WindingPolicy_Positive<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W = default_winding_t>
Polygon<T> PolygonSimplify_Positive2(const Polygon<T, W> &polygon) {
	SweepEngine<T, OutputPolicy_Keyhole<T>, WindingPolicy_Positive<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<typename T, typename W = default_winding_t>
Polygon<T> PolygonSimplify_Negative(const Polygon<T, W> &polygon) {
	SweepEngine<T, OutputPolicy_Simple<T>, WindingPolicy_Negative<W>> engine(polygon);
	engine.Process();
	return engine.Result();
}

}
