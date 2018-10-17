#pragma once

#include "Wrappers.h"

namespace PolyMathWrapper {

using namespace Wrappers;

double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);

};
