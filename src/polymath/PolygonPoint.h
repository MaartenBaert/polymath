#pragma once

#include "Common.h"

#include "NumericalEngine.h"
#include "Polygon.h"
#include "Vertex.h"

#include <limits>

namespace PolyMath {

template<typename T, BoundaryRule boundary_rule = BOUNDARYRULE_LAZY>
int64_t PolygonPointWindingNumber(const Polygon<T> &polygon, Vertex<T> point) {

	typedef PolyMath::NumericalEngine<T> NumericalEngine;

	struct Helpers {
		inline static bool CompareVertex(Vertex<T> a, Vertex<T> b) {
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
				const Vertex<T> &v1 = polygon.vertices[prev];
				const Vertex<T> &v2 = polygon.vertices[index];
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

template<class T>
T PolygonPointEdgeDistance(const Polygon<T> &polygon, Vertex<T> point) {

	T best = std::numeric_limits<T>::max();
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
			const Vertex<T> &v1 = polygon.vertices[prev];
			const Vertex<T> &v2 = polygon.vertices[index];
			T pos = (point.x - v2.x) * (v1.x - v2.x) + (point.y - v2.y) * (v1.y - v2.y);
			T len = Square(v1.x - v2.x) + Square(v1.y - v2.y);
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
