#include "GeosWrapper.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifndef USE_UNSTABLE_GEOS_CPP_API
#define USE_UNSTABLE_GEOS_CPP_API
#endif
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/PrecisionModel.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <cmath>

#include <chrono>

namespace GeosWrapper {

// Uses the GEOS C++ API as of GEOS 3.12 (ownership is passed with std::unique_ptr).

void PolyToGeos(const Polygon& input, std::unique_ptr<geos::geom::Geometry> &output, const geos::geom::GeometryFactory *factory) {
	std::vector<std::unique_ptr<geos::geom::Polygon>> polygons;
	std::unique_ptr<geos::geom::LinearRing> last_outer;
	std::vector<std::unique_ptr<geos::geom::LinearRing>> last_inner;
	auto Flush = [&]() -> void {
		if(last_outer != nullptr) {
			polygons.push_back(factory->createPolygon(std::move(last_outer), std::move(last_inner)));
			last_outer.reset();
			last_inner.clear();
		}
	};
	for(size_t i = 0; i < input.loops.size(); ++i) {
		const Vertex *vertices = input.GetLoopVertices(i);
		size_t n = input.GetLoopVertexCount(i);
		auto cl = std::make_unique<geos::geom::CoordinateSequence>();
		for(size_t j = 0; j < n; ++j) {
			cl->add(geos::geom::Coordinate(vertices[j].x, vertices[j].y));
		}
		cl->add(geos::geom::Coordinate(vertices[0].x, vertices[0].y));
		std::unique_ptr<geos::geom::LinearRing> lr = factory->createLinearRing(std::move(cl));
		if(input.loops[i].weight > 0) {
			Flush();
			last_outer = std::move(lr);
		} else {
			assert(last_outer != nullptr);
			last_inner.push_back(std::move(lr));
		}
	}
	Flush();
	output = factory->createMultiPolygon(std::move(polygons));
}

void PolyFromGeos(const std::unique_ptr<geos::geom::Geometry> &input, Polygon &output) {
	output.Clear();
	if(input == nullptr)
		return;
	// the result can be a Polygon or a MultiPolygon, both can be iterated with getGeometryN
	for(size_t i = 0; i < input->getNumGeometries(); ++i) {
		const geos::geom::Polygon *poly = dynamic_cast<const geos::geom::Polygon*>(input->getGeometryN(i));
		if(poly == nullptr)
			continue;
		{
			const geos::geom::LineString *ls = poly->getExteriorRing();
			for(size_t k = 0; k < ls->getNumPoints(); ++k) {
				const geos::geom::Coordinate& co = ls->getCoordinateN(k);
				output.AddVertex(Vertex(co.x, co.y));
			}
			output.AddLoopEnd(1);
		}
		for(size_t j = 0; j < poly->getNumInteriorRing(); ++j) {
			const geos::geom::LineString *ls = poly->getInteriorRingN(j);
			for(size_t k = 0; k < ls->getNumPoints(); ++k) {
				const geos::geom::Coordinate& co = ls->getCoordinateN(k);
				output.AddVertex(Vertex(co.x, co.y));
			}
			output.AddLoopEnd(-1);
		}
	}
}

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

	// init
	geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create();

	// import
	std::unique_ptr<geos::geom::Geometry> a, b, c;
	PolyToGeos(poly1, a, factory.get());
	PolyToGeos(poly2, b, factory.get());

	// benchmark
	bool error = false;
	auto t1 = std::chrono::high_resolution_clock::now();
	try {
		for(size_t loop = 0; loop < loops; ++loop) {
			c = a->Union(b.get());
		}
	} catch(...) {
		error = true;
	}
	auto t2 = std::chrono::high_resolution_clock::now();

	// export
	if(error) {
		result.Clear();
	} else {
		PolyFromGeos(c, result);
	}

	return (error)? 0.0 : std::chrono::duration<double>(t2 - t1).count() / double(loops);
}

}
