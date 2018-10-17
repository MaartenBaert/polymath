#include "PolyMathWrapper.h"

#include <cmath>

#include <chrono>

namespace PolyMathWrapper {

void AddPolygon(const Polygon &poly, Polygon &result) {
	for(size_t i = 0; i < poly.GetLoopCount(); i++) {
		const Vertex *vertices = poly.GetLoopVertices(i);
		size_t n = poly.GetLoopVertexCount(i);
		for(size_t j = 0; j < n; ++j) {
			result.AddVertex(vertices[j]);
		}
		result.AddLoopEnd(poly.GetLoopWindingWeight(i));
	}
}

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

	// import
	Polygon a, c;
	AddPolygon(poly1, a);
	AddPolygon(poly2, a);

	// benchmark
	auto t1 = std::chrono::high_resolution_clock::now();
	for(size_t loop = 0; loop < loops; ++loop) {
		c = PolyMath::Simplify(a);
	}
	auto t2 = std::chrono::high_resolution_clock::now();

	// export
	result = c;

	return std::chrono::duration<double>(t2 - t1).count() / double(loops);
}

}
