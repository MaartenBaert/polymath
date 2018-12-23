#pragma once

#include "Common.h"

#include "Polygon.h"
#include "PolygonPoint.h"
#include "SweepEngine.h"
#include "Vertex.h"
#include "Visualization.h"
#include "WindingEngine.h"

namespace PolyMath {

template<class Vertex, typename WindingNumber>
Polygon<Vertex> PolygonSimplify_NonZero(const Polygon<Vertex, WindingNumber> &polygon) {
	SweepEngine<Vertex, WindingEngine_NonZero<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<class Vertex, typename WindingNumber>
Polygon<Vertex> PolygonSimplify_EvenOdd(const Polygon<Vertex, WindingNumber> &polygon) {
	SweepEngine<Vertex, WindingEngine_EvenOdd<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<class Vertex, typename WindingNumber>
Polygon<Vertex> PolygonSimplify_Positive(const Polygon<Vertex, WindingNumber> &polygon) {
	SweepEngine<Vertex, WindingEngine_Positive<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

template<class Vertex, typename WindingNumber>
Polygon<Vertex> PolygonSimplify_Negative(const Polygon<Vertex, WindingNumber> &polygon) {
	SweepEngine<Vertex, WindingEngine_Negative<WindingNumber>> engine(polygon);
	engine.Process();
	return engine.Result();
}

}
