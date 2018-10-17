#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <vector>

#define POLYMATH_UNUSED(x) ((void) (x))

namespace PolyMath {

constexpr size_t INDEX_NONE = size_t(-1);

template<typename F>
F Square(F x) {
	return x * x;
}

}
