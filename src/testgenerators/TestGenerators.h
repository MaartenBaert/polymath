#pragma once

#include "TypeConverter.h"

#include "polymath/PolyMath.h"

namespace TestGenerators {

enum DualGridType {
	DUALGRID_DEFAULT,
	DUALGRID_STARS,
	DUALGRID_CIRCLES,
};

constexpr uint64_t SEED_MULT = UINT64_C(0x73f5256163293f15);
constexpr uint64_t SEED_ADD = UINT64_C(0x4a65a273f5acff8b);

void DualGrid(uint64_t seed, DualGridType type, uint32_t grid_size, double grid_angle, bool holes, Polygon results[2]);

Polygon Orthogonal(uint64_t seed, uint32_t num_lines, uint32_t num_polygons);

Polygon EdgeCases(uint64_t seed, uint32_t num_lines, uint32_t num_polygons, double eps);

Polygon Star(uint32_t num_points);

}
