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
Polygon<Vertex> Simplify(const Polygon<Vertex> &polygon) {
	SweepEngine<Vertex, WindingEngine_Union> engine(polygon);
	engine.Process(DummyVisualizationCallback);
	return engine.Result();
}

}
