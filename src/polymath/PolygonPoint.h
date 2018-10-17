#pragma once

#include "Common.h"
#include "Polygon.h"
#include "Vertex.h"

#include <limits>

namespace PolyMath {

template<class Vertex>
int64_t PolygonPointWindingNumber(const Polygon<Vertex> &polygon, Vertex point) {

	typedef typename Vertex::value_type value_type;

	int64_t winding_number = 0;
	size_t index = 0;
	for(size_t loop = 0; loop < polygon.GetLoopCount(); ++loop) {

		// get loop
		size_t begin = index;
		size_t end = polygon.GetLoopEnd(loop);
		int64_t winding_weight = polygon.GetLoopWindingWeight(loop);

		// ignore polygons with less than three vertices
		if(end - begin < 3) {
			index = end;
			continue;
		}

		// handle all vertices
		size_t prev = end - 1;
		bool prev_state = (polygon.GetVertex(prev).x() < point.x());
		for( ; index < end; ++index) {
			bool state = (polygon.GetVertex(index).x() < point.x());
			if(state != prev_state) {
				const Vertex &v1 = polygon.GetVertex(prev);
				const Vertex &v2 = polygon.GetVertex(index);
				value_type y = v1.y() + (v2.y() - v1.y()) * (point.x() - v1.x()) / (v2.x() - v1.x());
				if(y < point.y()) {
					if(state) {
						winding_number -= winding_weight;
					} else {
						winding_number += winding_weight;
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
typename Vertex::value_type PolygonPointEdgeDistance(const Polygon<Vertex> &polygon, Vertex point) {

	typedef typename Vertex::value_type value_type;

	value_type best = std::numeric_limits<value_type>::max();
	size_t index = 0;
	for(size_t loop = 0; loop < polygon.GetLoopCount(); ++loop) {

		// get loop
		size_t begin = index;
		size_t end = polygon.GetLoopEnd(loop);

		// ignore polygons without vertices
		if(begin == end) {
			index = end;
			continue;
		}

		// handle all vertices
		size_t prev = end - 1;
		for( ; index < end; ++index) {
			const Vertex &v1 = polygon.GetVertex(prev);
			const Vertex &v2 = polygon.GetVertex(index);
			value_type pos = (point.x() - v2.x()) * (v1.x() - v2.x()) + (point.y() - v2.y()) * (v1.y() - v2.y());
			value_type len = Square(v1.x() - v2.x()) + Square(v1.y() - v2.y());
			if(pos > 0.0 && pos < len) {
				best = std::min(best, Square((point.x() - v2.x()) * (v1.y() - v2.y()) - (point.y() - v2.y()) * (v1.x() - v2.x())) / len);
			} else {
				best = std::min(best, Square(point.x() - v2.x()) + Square(point.y() - v2.y()));
			}
			prev = index;
		}

	}

	return std::sqrt(best);
}

}
