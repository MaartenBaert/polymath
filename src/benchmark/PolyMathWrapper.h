#pragma once

#include "Wrappers.h"

namespace PolyMathWrapper {

using namespace Wrappers;

double BenchmarkUnion_I8(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_I16(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_I32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_I64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_F32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_F64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);

double BenchmarkUnion_S1(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);
double BenchmarkUnion_S2(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops);

};
