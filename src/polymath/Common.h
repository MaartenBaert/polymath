#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <vector>

#define POLYMATH_UNUSED(x) ((void) (x))

namespace PolyMath {

typedef int32_t default_winding_t;

enum BoundaryRule {
	BOUNDARYRULE_CLOSED,
	BOUNDARYRULE_OPEN,
	BOUNDARYRULE_CONSISTENT,
	BOUNDARYRULE_LAZY,
};

constexpr size_t INDEX_NONE = size_t(-1);

template<typename F>
F Square(F x) {
	return x * x;
}

}
