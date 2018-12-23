#include "ClipperWrapper.h"

#include "3rdparty/clipper.hpp"

#include <cmath>

#include <chrono>

namespace ClipperWrapper {

constexpr float CLIPPER_SCALE = 1e6;

void PolyToClipper(const Polygon &input, ClipperLib::Polygons &output) {
	output.clear();
	output.resize(input.GetLoopCount());
	for(size_t i = 0; i < input.GetLoopCount(); ++i) {
		const Vertex *vertices = input.GetLoopVertices(i);
		size_t n = input.GetLoopVertexCount(i);
		ClipperLib::Polygon &ring = output[i];
		ring.resize(n);
		if(input.GetLoopWindingWeight(i) > 0) {
			for(size_t j = 0; j < n; ++j) {
				ring[j] = ClipperLib::IntPoint(lrint(vertices[j].x * CLIPPER_SCALE), lrint(vertices[j].y * CLIPPER_SCALE));
			}
		} else {
			for(size_t j = 0; j < n; ++j) {
				ring[n - 1 - j] = ClipperLib::IntPoint(lrint(vertices[j].x * CLIPPER_SCALE), lrint(vertices[j].y * CLIPPER_SCALE));
			}
		}
	}
}

void PolyFromClipper(const ClipperLib::Polygons &input, Polygon &output) {
	output.Clear();
	for(size_t i = 0; i < input.size(); ++i) {
		const ClipperLib::Polygon &ring = input[i];
		for(size_t j = 0; j < ring.size(); ++j) {
			output.AddVertex(Vertex(float(ring[j].X) / CLIPPER_SCALE, float(ring[j].Y) / CLIPPER_SCALE));
		}
		output.AddLoopEnd(1);
	}
}

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

	// import
	ClipperLib::Polygons a, b, c;
	PolyToClipper(poly1, a);
	PolyToClipper(poly2, b);

	// benchmark
	auto t1 = std::chrono::high_resolution_clock::now();
	for(size_t loop = 0; loop < loops; ++loop) {
		c.clear();
		ClipperLib::Clipper cl;
		cl.AddPolygons(a, ClipperLib::ptSubject);
		cl.AddPolygons(b, ClipperLib::ptClip);
		cl.Execute(ClipperLib::ctUnion, c, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
	}
	auto t2 = std::chrono::high_resolution_clock::now();

	// export
	PolyFromClipper(c, result);

	return std::chrono::duration<double>(t2 - t1).count() / double(loops);
}

}
