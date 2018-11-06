#include "PolyMathWrapper.h"

#include <cmath>

#include <chrono>

namespace PolyMathWrapper {

template<typename F>
struct Conversion {

	typedef PolyMath::Vertex<F> Vertex2;
	typedef PolyMath::Polygon<Vertex2> Polygon2;

	static void PolyToPolyMath(const Polygon &input, Polygon2 &output) {
		for(size_t i = 0; i < input.GetLoopCount(); i++) {
			const Vertex *vertices = input.GetLoopVertices(i);
			size_t n = input.GetLoopVertexCount(i);
			for(size_t j = 0; j < n; ++j) {
				const Vertex &v = vertices[j];
				output.AddVertex(Vertex2(F(v.x()), F(v.y())));
			}
			output.AddLoopEnd(input.GetLoopWindingWeight(i));
		}
	}

	static void PolyFromPolyMath(const Polygon2 &input, Polygon &output) {
		for(size_t i = 0; i < input.GetLoopCount(); i++) {
			const Vertex2 *vertices = input.GetLoopVertices(i);
			size_t n = input.GetLoopVertexCount(i);
			for(size_t j = 0; j < n; ++j) {
				const Vertex2 &v = vertices[j];
				output.AddVertex(Vertex(v.x(), v.y()));
			}
			output.AddLoopEnd(input.GetLoopWindingWeight(i));
		}
	}

	static double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

		// import
		Polygon2 ab, c;
		PolyToPolyMath(poly1, ab);
		PolyToPolyMath(poly2, ab);

		// benchmark
		auto t1 = std::chrono::high_resolution_clock::now();
		for(size_t loop = 0; loop < loops; ++loop) {
			c = PolyMath::PolygonUnion(ab);
		}
		auto t2 = std::chrono::high_resolution_clock::now();

		// export
		result.Clear();
		PolyFromPolyMath(c, result);

		return std::chrono::duration<double>(t2 - t1).count() / double(loops);
	}

};

double BenchmarkUnion_F32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<float>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_F64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<double>::BenchmarkUnion(poly1, poly2, result, loops); }

}
