#include "Clipper2Wrapper.h"

#include "3rdparty/clipper2/clipper2.h"
#include "benchmark/3rdparty/clipper2/clipper2.core.h"

#include <cmath>

#include <chrono>

namespace Clipper2Wrapper {

constexpr float CLIPPER_SCALE = 1e6;

void PolyToClipper(const Polygon &input, Clipper2Lib::Paths64 &output) {
	output.clear();
	output.resize(input.loops.size());
	for(size_t i = 0; i < input.loops.size(); ++i) {
		const Vertex *vertices = input.GetLoopVertices(i);
		size_t n = input.GetLoopVertexCount(i);
		Clipper2Lib::Path64 &ring = output[i];
		ring.resize(n);
		if(input.loops[i].weight > 0) {
			for(size_t j = 0; j < n; ++j) {
				ring[j] = Clipper2Lib::Point64(lrint(vertices[j].x * CLIPPER_SCALE), lrint(vertices[j].y * CLIPPER_SCALE));
			}
		} else {
			for(size_t j = 0; j < n; ++j) {
				ring[n - 1 - j] = Clipper2Lib::Point64(lrint(vertices[j].x * CLIPPER_SCALE), lrint(vertices[j].y * CLIPPER_SCALE));
			}
		}
	}
}

void PolyFromClipper(const Clipper2Lib::Paths64 &input, Polygon &output) {
	output.Clear();
	for(size_t i = 0; i < input.size(); ++i) {
		const Clipper2Lib::Path64 &ring = input[i];
		for(size_t j = 0; j < ring.size(); ++j) {
			output.AddVertex(Vertex(float(ring[j].x) / CLIPPER_SCALE, float(ring[j].y) / CLIPPER_SCALE));
		}
		output.AddLoopEnd(1);
	}
}

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

	// import
	Clipper2Lib::Paths64 a, b, c;
	PolyToClipper(poly1, a);
	PolyToClipper(poly2, b);

	// benchmark
	auto t1 = std::chrono::high_resolution_clock::now();
	for(size_t loop = 0; loop < loops; ++loop) {
		c.clear();
		c = Clipper2Lib::Union(a, b, Clipper2Lib::FillRule::NonZero);
	}
	auto t2 = std::chrono::high_resolution_clock::now();

	// export
	PolyFromClipper(c, result);

	return std::chrono::duration<double>(t2 - t1).count() / double(loops);
}

}
