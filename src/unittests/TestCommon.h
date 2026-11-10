#pragma once

// Helpers shared by the output tests.

#include "polymath/PolyMath.h"
#include "testgenerators/TestGenerators.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace TestCommon {

enum OutputType {
	OUTPUTTYPE_SIMPLE,
	OUTPUTTYPE_KEYHOLE,
	OUTPUTTYPE_MONOTONE,
	OUTPUTTYPE_TRIANGLES,
};
constexpr OutputType ALL_OUTPUT_TYPES[] = {OUTPUTTYPE_SIMPLE, OUTPUTTYPE_KEYHOLE, OUTPUTTYPE_MONOTONE, OUTPUTTYPE_TRIANGLES};
inline const char* OutputTypeName(OutputType output) {
	switch(output) {
		case OUTPUTTYPE_SIMPLE: return "simple";
		case OUTPUTTYPE_KEYHOLE: return "keyhole";
		case OUTPUTTYPE_MONOTONE: return "monotone";
		case OUTPUTTYPE_TRIANGLES: return "triangles";
	}
	return "?";
}

constexpr PolyMath::WindingRule ALL_WINDING_RULES[] = {
	PolyMath::WINDINGRULE_NONZERO, PolyMath::WINDINGRULE_EVENODD, PolyMath::WINDINGRULE_POSITIVE, PolyMath::WINDINGRULE_NEGATIVE,
};
inline bool EvaluateWindingRule(PolyMath::WindingRule rule, int64_t winding_number) {
	switch(rule) {
		case PolyMath::WINDINGRULE_NONZERO: return (winding_number != 0);
		case PolyMath::WINDINGRULE_EVENODD: return bool(winding_number & 1);
		case PolyMath::WINDINGRULE_POSITIVE: return (winding_number > 0);
		case PolyMath::WINDINGRULE_NEGATIVE: return (winding_number < 0);
	}
	return false;
}

template<typename T, class OutputPolicy>
PolyMath::Polygon<T> Run2(const PolyMath::Polygon<T> &input, PolyMath::WindingRule rule) {
	PolyMath::SweepEngine<T, OutputPolicy, PolyMath::WindingPolicy_Dynamic<>, PolyMath::SweepTree_Basic2> engine(
		input, OutputPolicy(), PolyMath::WindingPolicy_Dynamic<>(rule));
	engine.Process();
	return engine.Result();
}

template<typename T>
PolyMath::Polygon<T> Run(const PolyMath::Polygon<T> &input, OutputType output, PolyMath::WindingRule rule) {
	switch(output) {
		case OUTPUTTYPE_SIMPLE: return Run2<T, PolyMath::OutputPolicy_Simple<T>>(input, rule);
		case OUTPUTTYPE_KEYHOLE: return Run2<T, PolyMath::OutputPolicy_Keyhole<T>>(input, rule);
		case OUTPUTTYPE_MONOTONE: return Run2<T, PolyMath::OutputPolicy_Monotone<T>>(input, rule);
		case OUTPUTTYPE_TRIANGLES: return Run2<T, PolyMath::OutputPolicy_Triangles<T>>(input, rule);
	}
	return PolyMath::Polygon<T>();
}

// Sum of the signed areas of all loops, multiplied by their winding weights.
template<typename T>
double WeightedArea(const PolyMath::Polygon<T> &polygon) {
	double area = 0.0;
	for(size_t i = 0; i < polygon.loops.size(); ++i) {
		const PolyMath::Vertex<T> *v = polygon.GetLoopVertices(i);
		size_t n = polygon.GetLoopVertexCount(i);
		double loop_area = 0.0;
		for(size_t j = 0; j < n; ++j) {
			const PolyMath::Vertex<T> &a = v[j], &b = v[(j + 1) % n];
			loop_area += 0.5 * (double(a.x) * double(b.y) - double(b.x) * double(a.y));
		}
		area += loop_area * double(polygon.loops[i].weight);
	}
	return area;
}

// Named test inputs, generated with the test generators.
struct TestInput {
	std::string m_name;
	TestGenerators::Polygon m_polygon;
};

// The input set used by the golden tests. Changing it requires regenerating the golden data.
// The edge case inputs depend on the epsilon of the type, so the type is a parameter.
template<typename T>
std::vector<TestInput> GoldenInputs() {
	std::vector<TestInput> inputs;
	const char *grid_type_names[] = {"default", "stars", "circles"};
	TestGenerators::DoubleGridType grid_types[] = {
		TestGenerators::DOUBLEGRID_DEFAULT, TestGenerators::DOUBLEGRID_STARS, TestGenerators::DOUBLEGRID_CIRCLES,
	};
	for(size_t type = 0; type < 3; ++type) {
		for(uint32_t size : {1, 3, 10}) {
			for(bool holes : {false, true}) {
				for(uint64_t seed = 0; seed < 4; ++seed) {
					TestGenerators::Polygon polys[2];
					TestGenerators::DoubleGrid(seed, grid_types[type], size, 20.0, holes, polys);
					TestGenerators::Polygon poly = polys[0];
					poly += polys[1];
					inputs.push_back({"doublegrid-" + std::string(grid_type_names[type]) + "-size" + std::to_string(size) +
						"-holes" + std::to_string(int(holes)) + "-seed" + std::to_string(seed), poly});
				}
			}
		}
	}
	for(uint64_t seed = 0; seed < 10; ++seed) {
		inputs.push_back({"orthogonal-seed" + std::to_string(seed), TestGenerators::Orthogonal(seed, 10, 5)});
	}
	double eps = TestGenerators::TypeConverter<T>::Epsilon();
	for(uint64_t seed = 0; seed < 50; ++seed) {
		inputs.push_back({"edgecases-seed" + std::to_string(seed), TestGenerators::EdgeCases(seed, 5, 10, 2.0 * eps)});
	}
	for(uint32_t points : {5, 7, 13}) {
		for(double angle : {0.0, 7.3}) {
			inputs.push_back({"star-points" + std::to_string(points) + "-angle" + std::to_string(angle),
				TestGenerators::Star(points, angle)});
		}
	}
	return inputs;
}

// 64-bit FNV-1a hash.
class Hasher {
private:
	uint64_t m_hash = UINT64_C(0xcbf29ce484222325);
public:
	template<typename V>
	void Add(V value) {
		static_assert(std::is_integral<V>::value, "only integers can be hashed deterministically");
		uint64_t x = uint64_t(value);
		for(size_t i = 0; i < sizeof(V); ++i) {
			m_hash ^= (x >> (8 * i)) & 0xff;
			m_hash *= UINT64_C(0x100000001b3);
		}
	}
	uint64_t Get() const { return m_hash; }
};

// Hashes a polygon in a canonical form that only depends on the geometry and orientation of the loops, not on the
// start vertex of each loop or the order of the loops. Integer types only, since floating point results are not
// guaranteed to be identical across platforms and compiler options.
template<typename T>
void HashCanonical(Hasher &hasher, const PolyMath::Polygon<T> &polygon) {
	static_assert(std::is_integral<T>::value, "only integer polygons can be hashed deterministically");
	typedef std::pair<T, T> Point;
	struct Loop {
		int64_t m_weight;
		std::vector<Point> m_points;
		bool operator<(const Loop &other) const {
			if(m_weight != other.m_weight)
				return m_weight < other.m_weight;
			return m_points < other.m_points;
		}
	};
	std::vector<Loop> loops(polygon.loops.size());
	for(size_t i = 0; i < polygon.loops.size(); ++i) {
		const PolyMath::Vertex<T> *v = polygon.GetLoopVertices(i);
		size_t n = polygon.GetLoopVertexCount(i);
		std::vector<Point> points(n);
		for(size_t j = 0; j < n; ++j) {
			points[j] = Point(v[j].x, v[j].y);
		}
		// pick the lexicographically smallest rotation (the loop direction is preserved)
		std::vector<Point> best;
		if(n != 0) {
			Point min_point = *std::min_element(points.begin(), points.end());
			for(size_t start = 0; start < n; ++start) {
				if(points[start] != min_point)
					continue;
				std::vector<Point> rotated(points.begin() + ptrdiff_t(start), points.end());
				rotated.insert(rotated.end(), points.begin(), points.begin() + ptrdiff_t(start));
				if(best.empty() || rotated < best)
					best = std::move(rotated);
			}
		}
		loops[i].m_weight = int64_t(polygon.loops[i].weight);
		loops[i].m_points = std::move(best);
	}
	std::sort(loops.begin(), loops.end());
	hasher.Add(uint64_t(loops.size()));
	for(const Loop &loop : loops) {
		hasher.Add(loop.m_weight);
		hasher.Add(uint64_t(loop.m_points.size()));
		for(const Point &p : loop.m_points) {
			hasher.Add(int64_t(p.first));
			hasher.Add(int64_t(p.second));
		}
	}
}

}
