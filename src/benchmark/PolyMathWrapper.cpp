#include "PolyMathWrapper.h"

#include "testgenerators/TypeConverter.h"

#include <cmath>

#include <chrono>

namespace PolyMathWrapper {

template<typename T>
struct Conversion {

	typedef PolyMath::Vertex<T> Vertex2;
	typedef PolyMath::Polygon<T> Polygon2;

	static double BenchmarkUnion(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) {

		// import
		Polygon2 ab, c;
		ab += TestGenerators::TypeConverter<T>::ConvertPolygonToType(poly1);
		ab += TestGenerators::TypeConverter<T>::ConvertPolygonToType(poly2);

		// benchmark
		auto t1 = std::chrono::high_resolution_clock::now();
		for(size_t loop = 0; loop < loops; ++loop) {
			c = PolyMath::PolygonSimplify_Positive(ab);
		}
		auto t2 = std::chrono::high_resolution_clock::now();

		// export
		result = TestGenerators::TypeConverter<T>::ConvertPolygonFromType(c);

		return std::chrono::duration<double>(t2 - t1).count() / double(loops);
	}

};

double BenchmarkUnion_I8(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<int8_t>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_I16(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<int16_t>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_I32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<int32_t>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_I64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<int64_t>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_F32(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<float>::BenchmarkUnion(poly1, poly2, result, loops); }
double BenchmarkUnion_F64(const Polygon &poly1, const Polygon &poly2, Polygon &result, size_t loops) { return Conversion<double>::BenchmarkUnion(poly1, poly2, result, loops); }

}
