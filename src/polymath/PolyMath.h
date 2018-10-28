#pragma once

#include "Common.h"
#include "Polygon.h"
#include "PolygonPoint.h"
#include "SweepEngine.h"
#include "Vertex.h"
#include "Visualization.h"
#include "WindingEngine.h"

namespace PolyMath {

template<class Vertex>
Polygon<Vertex> PolygonUnion(const Polygon<Vertex> &polygon) {
	SweepEngine<Vertex, NumericalEngine<typename Vertex::value_type>, WindingEngine_Union> engine(polygon);
	engine.Process(DummyVisualizationCallback);
	return engine.Result();
}

template<class Vertex>
Polygon<Vertex> PolygonIntersection(const Polygon<Vertex> &polygon) {
	SweepEngine<Vertex, NumericalEngine<typename Vertex::value_type>, WindingEngine_Intersection> engine(polygon);
	engine.Process(DummyVisualizationCallback);
	return engine.Result();
}

template<class Vertex>
Polygon<Vertex> PolygonXor(const Polygon<Vertex> &polygon) {
	SweepEngine<Vertex, NumericalEngine<typename Vertex::value_type>, WindingEngine_Xor> engine(polygon);
	engine.Process(DummyVisualizationCallback);
	return engine.Result();
}

}
