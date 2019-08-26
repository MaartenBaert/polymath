#pragma once

#include "Common.h"

#include "NumericalEngine.h"
#include "Polygon.h"
#include "Vertex.h"

#include <limits>

namespace PolyMath {

template<class Vertex, BoundaryRule boundary_rule = BOUNDARYRULE_LAZY>
int64_t PolygonPointWindingNumber(const Polygon<Vertex> &polygon, Vertex point) {

	typedef PolyMath::NumericalEngine<typename Vertex::ValueType> NumericalEngine;

	struct Helpers {
		inline static bool CompareVertex(Vertex a, Vertex b) {
			if(boundary_rule == BOUNDARYRULE_CLOSED || boundary_rule == BOUNDARYRULE_OPEN) {
				if(a.x < b.x)
					return true;
				if(a.x > b.x)
					return false;
				return (a.y < b.y);
			} else {
				return (a.x < b.x);
			}
		}
	};

	int64_t winding_number = 0;
	size_t index = 0;
	for(size_t loop = 0; loop < polygon.loops.size(); ++loop) {

		// get loop
		size_t begin = index;
		size_t end = polygon.loops[loop].end;
		int64_t winding_weight = polygon.loops[loop].weight;

		// ignore polygons with less than two/three vertices
		if(end - begin < 3) {
			index = end;
			continue;
		}

		// handle all vertices
		size_t prev = end - 1;
		bool prev_state = Helpers::CompareVertex(point, polygon.vertices[prev]);
		for( ; index < end; ++index) {
			bool state = Helpers::CompareVertex(point, polygon.vertices[index]);
			if(state != prev_state) {
				const Vertex &v1 = polygon.vertices[prev];
				const Vertex &v2 = polygon.vertices[index];
				bool strict = (boundary_rule == BOUNDARYRULE_OPEN || (boundary_rule == BOUNDARYRULE_CONSISTENT && !state));
				if(NumericalEngine::OrientationTest(v1.x, v1.y, v2.x, v2.y, point.x, point.y, strict) == state) {
					if(state) {
						winding_number += winding_weight;
					} else {
						winding_number -= winding_weight;
					}
				}
			}
			prev = index;
			prev_state = state;
		}

	}

	return winding_number;
}

template<class Vertex>
typename Vertex::ValueType PolygonPointEdgeDistance(const Polygon<Vertex> &polygon, Vertex point) {

	typedef typename Vertex::ValueType value_type;

	value_type best = std::numeric_limits<value_type>::max();
	size_t index = 0;
	for(size_t loop = 0; loop < polygon.loops.size(); ++loop) {

		// get loop
		size_t begin = index;
		size_t end = polygon.loops[loop].end;

		// ignore polygons without vertices
		if(begin == end) {
			index = end;
			continue;
		}

		// handle all vertices
		size_t prev = end - 1;
		for( ; index < end; ++index) {
			const Vertex &v1 = polygon.vertices[prev];
			const Vertex &v2 = polygon.vertices[index];
			value_type pos = (point.x - v2.x) * (v1.x - v2.x) + (point.y - v2.y) * (v1.y - v2.y);
			value_type len = Square(v1.x - v2.x) + Square(v1.y - v2.y);
			if(pos > 0.0 && pos < len) {
				best = std::min(best, Square((point.x - v2.x) * (v1.y - v2.y) - (point.y - v2.y) * (v1.x - v2.x)) / len);
			} else {
				best = std::min(best, Square(point.x - v2.x) + Square(point.y - v2.y));
			}
			prev = index;
		}

	}

	return std::sqrt(best);
}

}
