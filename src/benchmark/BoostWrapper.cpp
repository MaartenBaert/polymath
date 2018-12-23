#include "BoostWrapper.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <chrono>

namespace BoostWrapper {

template<typename F>
struct Conversion {

	typedef boost::geometry::model::d2::point_xy<F> boost_point;
	typedef boost::geometry::model::ring<boost_point> boost_ring;
	typedef boost::geometry::model::polygon<boost_point> boost_polygon;
	typedef boost::geometry::model::multi_polygon<boost_polygon> boost_polygons;

	static void PolyToBoost(const Polygon &input, boost_polygons &output) {
		output.clear();
		output.resize(input.GetLoopCount());
		size_t num = 0;
		for(size_t i = 0; i < input.GetLoopCount(); ++i) {
			const Vertex *vertices = input.GetLoopVertices(i);
			size_t n = input.GetLoopVertexCount(i);
			if(input.GetLoopWindingWeight(i) > 0) {
				boost_ring &ring = output[num++].outer();
				ring.resize(n);
				for(size_t j = 0; j < n; ++j) {
					ring[j] = boost_point(F(vertices[j].x), F(vertices[j].y));
				}
			} else {
				boost_ring ring;
				ring.resize(n);
				for(size_t j = 0; j < n; ++j) {
					ring[j] = boost_point(F(vertices[j].x), F(vertices[j].y));
				}
				assert(num > 0);
				output[num - 1].inners().push_back(ring);
			}
		}
		output.resize(num);
		for(size_t i = 0; i < num; ++i) {
			boost::geometry::correct(output[i]);
		}
	}

	static void PolyFromBoost(const boost_polygons &input, Polygon &output) {
		output.Clear();
		for(size_t i = 0; i < input.size(); ++i) {
			const boost_ring &ring = input[i].outer();
			for(size_t j = 0; j < ring.size(); ++j) {
				output.AddVertex(Vertex(ring[j].x(), ring[j].y()));
			}
			output.AddLoopEnd(1);
			for(size_t k = 0; k < input[i].inners().size(); ++k) {
				const boost_ring &ring = input[i].inners()[k];
				for(size_t j = 0; j < ring.size(); ++j) {
					output.AddVertex(Vertex(ring[j].x(), ring[j].y()));
				}
				output.AddLoopEnd(1);
			}
		}
	}

	static double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

		// import
		boost_polygons a, b, c;
		PolyToBoost(poly1, a);
		PolyToBoost(poly2, b);

		// benchmark
		auto t1 = std::chrono::high_resolution_clock::now();
		for(size_t loop = 0; loop < loops; ++loop) {
			c.clear();
			boost::geometry::union_(a, b, c);
		}
		auto t2 = std::chrono::high_resolution_clock::now();

		// export
		PolyFromBoost(c, result);

		return std::chrono::duration<double>(t2 - t1).count() / double(loops);
	}

};

double BenchmarkUnion_F32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<float>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_F64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<double>::BenchmarkUnion(poly1, poly2, result, loops); }

}
