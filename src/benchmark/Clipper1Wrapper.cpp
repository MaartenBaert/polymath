#include "Clipper1Wrapper.h"

#include "3rdparty/clipper1/clipper1.hpp"

#include <cmath>

#include <chrono>

namespace Clipper1Wrapper {

constexpr float CLIPPER_SCALE = 1e6;

void PolyToClipper(const Polygon &input, Clipper1Lib::Polygons &output) {
	output.clear();
	output.resize(input.loops.size());
	for(size_t i = 0; i < input.loops.size(); ++i) {
		const Vertex *vertices = input.GetLoopVertices(i);
		size_t n = input.GetLoopVertexCount(i);
		Clipper1Lib::Polygon &ring = output[i];
		ring.resize(n);
		if(input.loops[i].weight > 0) {
			for(size_t j = 0; j < n; ++j) {
				ring[j] = Clipper1Lib::IntPoint(lrint(vertices[j].x * CLIPPER_SCALE), lrint(vertices[j].y * CLIPPER_SCALE));
			}
		} else {
			for(size_t j = 0; j < n; ++j) {
				ring[n - 1 - j] = Clipper1Lib::IntPoint(lrint(vertices[j].x * CLIPPER_SCALE), lrint(vertices[j].y * CLIPPER_SCALE));
			}
		}
	}
}

void PolyFromClipper(const Clipper1Lib::Polygons &input, Polygon &output) {
	output.Clear();
	for(size_t i = 0; i < input.size(); ++i) {
		const Clipper1Lib::Polygon &ring = input[i];
		for(size_t j = 0; j < ring.size(); ++j) {
			output.AddVertex(Vertex(float(ring[j].X) / CLIPPER_SCALE, float(ring[j].Y) / CLIPPER_SCALE));
		}
		output.AddLoopEnd(1);
	}
}

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

	// import
	Clipper1Lib::Polygons a, b, c;
	PolyToClipper(poly1, a);
	PolyToClipper(poly2, b);

	// benchmark
	auto t1 = std::chrono::high_resolution_clock::now();
	for(size_t loop = 0; loop < loops; ++loop) {
		c.clear();
		Clipper1Lib::Clipper cl;
		cl.AddPolygons(a, Clipper1Lib::ptSubject);
		cl.AddPolygons(b, Clipper1Lib::ptClip);
		cl.Execute(Clipper1Lib::ctUnion, c, Clipper1Lib::pftNonZero, Clipper1Lib::pftNonZero);
	}
	auto t2 = std::chrono::high_resolution_clock::now();

	// export
	PolyFromClipper(c, result);

	return std::chrono::duration<double>(t2 - t1).count() / double(loops);
}

}
