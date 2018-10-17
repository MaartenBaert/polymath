#pragma once

#include "PolyMath.h"

namespace TestGenerators {

typedef PolyMath::Vertex<double> Vertex;
typedef PolyMath::Polygon<Vertex> Polygon;
typedef PolyMath::Visualization<Vertex> Visualization;

enum DualGridType {
	DUALGRID_DEFAULT,
	DUALGRID_STARS,
	DUALGRID_CIRCLES,
};

constexpr uint64_t SEED_MULT = UINT64_C(0x73f5256163293f15);
constexpr uint64_t SEED_ADD = UINT64_C(0x4a65a273f5acff8b);

void DualGridTest(uint64_t seed, uint32_t grid_size, double grid_angle, DualGridType type, bool holes, Polygon *results);
Polygon EdgeCaseTest(uint64_t seed, uint32_t num_lines, uint32_t num_polygons);

}
