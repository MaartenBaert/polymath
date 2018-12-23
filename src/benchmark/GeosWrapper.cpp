#include "GeosWrapper.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define USE_UNSTABLE_GEOS_CPP_API
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
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

void PolyToGeos(const Polygon& input, geos::geom::MultiPolygon *&output, geos::geom::GeometryFactory *factory) {
	geos::geom::LinearRing* last_outer = nullptr;
	std::vector<geos::geom::Geometry*> *last_inner = nullptr;
	std::vector<geos::geom::Geometry*> *polygons = new std::vector<geos::geom::Geometry*>();
	auto Flush = [&]() -> void {
		if(last_outer != nullptr) {
			geos::geom::Polygon *poly = factory->createPolygon(last_outer, last_inner);
			polygons->push_back(poly);
			last_outer = nullptr;
			last_inner = nullptr;
		}
	};
	for(size_t i = 0; i < input.GetLoopCount(); ++i) {
		const Vertex *vertices = input.GetLoopVertices(i);
		size_t n = input.GetLoopVertexCount(i);
		geos::geom::CoordinateSequence *cl = new geos::geom::CoordinateArraySequence();
		for(size_t j = 0; j < n; ++j) {
			cl->add(geos::geom::Coordinate(vertices[j].x, vertices[j].y));
		}
		cl->add(geos::geom::Coordinate(vertices[0].x, vertices[0].y));
		geos::geom::LinearRing *lr = factory->createLinearRing(cl);
		if(input.GetLoopWindingWeight(i) > 0) {
			Flush();
			last_outer = lr;
		} else {
			assert(last_outer != nullptr);
			if(last_inner == nullptr)
				last_inner = new std::vector<geos::geom::Geometry*>();
			last_inner->push_back(lr);
		}
	}
	Flush();
	output = factory->createMultiPolygon(polygons);
}

void PolyFromGeos(geos::geom::MultiPolygon *input, Polygon &output) {
	output.Clear();
	if(input == nullptr)
		return;
	for(size_t i = 0; i < input->getNumGeometries(); ++i) {
		const geos::geom::Polygon *poly = dynamic_cast<const geos::geom::Polygon*>(input->getGeometryN(i));
		{
			const geos::geom::LineString *ls = poly->getExteriorRing();
			for(size_t k = 0; k < ls->getNumPoints(); ++k) {
				const geos::geom::Coordinate& co = ls->getCoordinateN(int(k));
				output.AddVertex(Vertex(co.x, co.y));
			}
			output.AddLoopEnd(1);
		}
		for(size_t j = 0; j < poly->getNumInteriorRing(); ++j) {
			const geos::geom::LineString *ls = poly->getInteriorRingN(j);
			for(size_t k = 0; k < ls->getNumPoints(); ++k) {
				const geos::geom::Coordinate& co = ls->getCoordinateN(int(k));
				output.AddVertex(Vertex(co.x, co.y));
			}
			output.AddLoopEnd(-1);
		}
	}
}

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

	// init
	geos::geom::PrecisionModel *pm = new geos::geom::PrecisionModel(geos::geom::PrecisionModel::FLOATING);
	geos::geom::GeometryFactory::Ptr factory = geos::geom::GeometryFactory::create(pm, -1);
	delete pm;

	// import
	geos::geom::MultiPolygon *a = nullptr, *b = nullptr, *c = nullptr;
	PolyToGeos(poly1, a, factory.get());
	PolyToGeos(poly2, b, factory.get());

	// benchmark
	bool error = false;
	auto t1 = std::chrono::high_resolution_clock::now();
	try {
		for(size_t loop = 0; loop < loops; ++loop) {
			delete c;
			c = nullptr;
			c = dynamic_cast<geos::geom::MultiPolygon*>(a->Union(b));
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

	// free
	delete a;
	delete b;
	delete c;

	return (error)? 0.0 : std::chrono::duration<double>(t2 - t1).count() / double(loops);
}

}
