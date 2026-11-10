// Property tests for the output of all output policies:
// - The output uses the same orientation convention as the input, so it can be used directly as input for another
//   operation with the same winding rule.
// - At points that are not close to any edge, the winding number of the output is exactly 1 where the input is
//   inside according to the winding rule, and 0 elsewhere.

#include "TestCommon.h"

#include "3rdparty/catch.hpp"

#include <random>

using namespace TestCommon;

namespace {

template<typename T>
PolyMath::Polygon<T> SquareWithHole() {
	// The outer square has the positive orientation (counter-clockwise if the Y axis points up). The hole has the same
	// orientation but a negative weight.
	PolyMath::Polygon<T> polygon;
	auto add_square = [&](T a, T b, int32_t weight) {
		polygon.AddVertex(PolyMath::Vertex<T>(a, a));
		polygon.AddVertex(PolyMath::Vertex<T>(b, a));
		polygon.AddVertex(PolyMath::Vertex<T>(b, b));
		polygon.AddVertex(PolyMath::Vertex<T>(a, b));
		polygon.AddLoopEnd(weight);
	};
	add_square(T(0), T(10), 1);
	add_square(T(3), T(7), -1);
	return polygon;
}

// Runs the given output policy with the 'positive' rule, then feeds the result back into a simple union with the same
// rule. Both should produce the reference area.
template<typename T>
void CheckChaining(const PolyMath::Polygon<T> &input, double reference_area, double tolerance) {
	for(OutputType output_type : ALL_OUTPUT_TYPES) {
		INFO("output: " << OutputTypeName(output_type));
		PolyMath::Polygon<T> output = Run(input, output_type, PolyMath::WINDINGRULE_POSITIVE);
		CHECK(WeightedArea(output) == Approx(reference_area).epsilon(tolerance));
		PolyMath::Polygon<T> refed = Run(output, OUTPUTTYPE_SIMPLE, PolyMath::WINDINGRULE_POSITIVE);
		CHECK(WeightedArea(refed) == Approx(reference_area).epsilon(tolerance));
		for(size_t i = 0; i < output.loops.size(); ++i) {
			CHECK(output.loops[i].weight == 1);
		}
	}
	// every individual triangle should have the positive orientation
	PolyMath::Polygon<T> triangles = Run(input, OUTPUTTYPE_TRIANGLES, PolyMath::WINDINGRULE_POSITIVE);
	for(size_t i = 0; i < triangles.loops.size(); ++i) {
		PolyMath::Polygon<T> single;
		const PolyMath::Vertex<T> *v = triangles.GetLoopVertices(i);
		for(size_t j = 0; j < triangles.GetLoopVertexCount(i); ++j) {
			single.AddVertex(v[j]);
		}
		single.AddLoopEnd(1);
		double area = WeightedArea(single);
		if(area < 0.0) { // for speed
			REQUIRE(area >= 0.0);
		}
	}
}

// Probes random points that are not close to any input or output edge. The probing is done in double precision on
// polygons converted back from type T, because PolygonPointEdgeDistance overflows for large integer coordinates.
template<typename T>
void CheckProbes(const TestInput &input, std::mt19937_64 &rng, size_t num_probes, size_t &probes_total, size_t &probes_used) {
	typedef TestGenerators::TypeConverter<T> Converter;
	PolyMath::Polygon<T> input_conv = Converter::ConvertPolygonToType(input.m_polygon);
	TestGenerators::Polygon input_rounded = Converter::ConvertPolygonFromType(input_conv);
	if(input_rounded.vertices.empty())
		return;
	double margin = std::max(64.0 * Converter::Epsilon(), 1e-12);
	double x_min = input_rounded.vertices[0].x, x_max = x_min, y_min = input_rounded.vertices[0].y, y_max = y_min;
	for(const TestGenerators::Vertex &v : input_rounded.vertices) {
		x_min = std::min(x_min, v.x); x_max = std::max(x_max, v.x);
		y_min = std::min(y_min, v.y); y_max = std::max(y_max, v.y);
	}
	std::uniform_real_distribution<double> dist_x{x_min, x_max}, dist_y{y_min, y_max};
	for(PolyMath::WindingRule rule : ALL_WINDING_RULES) {
		for(OutputType output_type : ALL_OUTPUT_TYPES) {
			TestGenerators::Polygon output = Converter::ConvertPolygonFromType(Run(input_conv, output_type, rule));
			for(size_t i = 0; i < num_probes; ++i) {
				TestGenerators::Vertex point(dist_x(rng), dist_y(rng));
				++probes_total;
				if(PolyMath::PolygonPointEdgeDistance(input_rounded, point) < margin ||
						(!output.vertices.empty() && PolyMath::PolygonPointEdgeDistance(output, point) < margin)) {
					continue;
				}
				++probes_used;
				int64_t expected = (EvaluateWindingRule(rule, PolyMath::PolygonPointWindingNumber(input_rounded, point)))? 1 : 0;
				int64_t actual = PolyMath::PolygonPointWindingNumber(output, point);
				if(actual != expected) { // for speed
					INFO("input: " << input.m_name << ", output: " << OutputTypeName(output_type) << ", rule: " << int(rule) <<
						", point: " << point);
					REQUIRE(actual == expected);
				}
			}
		}
	}
}

template<typename T>
void CheckProbesAllInputs() {
	std::mt19937_64 rng(UINT64_C(0x5be8d8a6e7e3c3b1));
	size_t probes_total = 0, probes_used = 0;
	for(const TestInput &input : GoldenInputs<T>()) {
		CheckProbes<T>(input, rng, 50, probes_total, probes_used);
	}
	// make sure that most probes are actually used, rather than skipped because they are too close to an edge
	INFO("probes used: " << probes_used << " / " << probes_total);
	CHECK(probes_used * 10 > probes_total * 9);
}

}

TEST_CASE("Output orientation, square with hole", "[output]") {
	CheckChaining<float>(SquareWithHole<float>(), 84.0, 1e-6);
	CheckChaining<int32_t>(SquareWithHole<int32_t>(), 84.0, 1e-9);
	CheckChaining<int64_t>(SquareWithHole<int64_t>(), 84.0, 1e-9);
}

TEST_CASE("Output orientation, random grids", "[output]") {
	for(uint64_t seed = 0; seed < 20; ++seed) {
		TestGenerators::Polygon inputs[2];
		TestGenerators::DoubleGrid(seed, TestGenerators::DOUBLEGRID_DEFAULT, 10, 20.0, true, inputs);
		PolyMath::Polygon<float> input = TestGenerators::TypeConverter<float>::ConvertPolygonToType(inputs[0]);
		input += TestGenerators::TypeConverter<float>::ConvertPolygonToType(inputs[1]);
		double reference_area = WeightedArea(Run(input, OUTPUTTYPE_SIMPLE, PolyMath::WINDINGRULE_POSITIVE));
		REQUIRE(reference_area > 0.0);
		CheckChaining<float>(input, reference_area, 1e-4);
	}
}

TEST_CASE("Output winding numbers, int32", "[output]") {
	CheckProbesAllInputs<int32_t>();
}

TEST_CASE("Output winding numbers, int64", "[output]") {
	CheckProbesAllInputs<int64_t>();
}

TEST_CASE("Output winding numbers, float", "[output]") {
	CheckProbesAllInputs<float>();
}
