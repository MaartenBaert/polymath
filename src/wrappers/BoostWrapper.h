#pragma once

#include "Wrappers.h"

namespace BoostWrapper {

using namespace Wrappers;

double BenchmarkUnion_F32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_F64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);

};
