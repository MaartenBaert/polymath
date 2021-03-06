#include "PolyMathWrapper.h"

#if BENCHMARK_WITH_BOOST
#include "BoostWrapper.h"
#endif

#if BENCHMARK_WITH_CLIPPER
#include "ClipperWrapper.h"
#endif

#if BENCHMARK_WITH_GEOS
#include "GeosWrapper.h"
#endif

#include "polymath/PolyMath.h"
#include "testgenerators/TestGenerators.h"

#include <iomanip>
#include <iostream>

struct Benchmark {
	std::string m_name;
	double (*m_func)(const TestGenerators::Polygon&, const TestGenerators::Polygon&, TestGenerators::Polygon&, size_t);
	bool m_run;
};

int main(int argc, char *argv[]) {
	POLYMATH_UNUSED(argc);
	POLYMATH_UNUSED(argv);

	std::vector<Benchmark> benchmarks = {
		//{"PolyMath I8" , PolyMathWrapper::BenchmarkUnion_I8 , true},
		//{"PolyMath I16", PolyMathWrapper::BenchmarkUnion_I16, true},
		//{"PolyMath I32", PolyMathWrapper::BenchmarkUnion_I32, true},
		//{"PolyMath I64", PolyMathWrapper::BenchmarkUnion_I64, true},
		//{"PolyMath F32", PolyMathWrapper::BenchmarkUnion_F32, true},
		//{"PolyMath F64", PolyMathWrapper::BenchmarkUnion_F64, true},
		{"PolyMath S1", PolyMathWrapper::BenchmarkUnion_S1, true},
		{"PolyMath S2", PolyMathWrapper::BenchmarkUnion_S2, true},
#if BENCHMARK_WITH_BOOST
		{"Boost F32"   , BoostWrapper   ::BenchmarkUnion_F32, true},
		//{"Boost F64"   , BoostWrapper   ::BenchmarkUnion_F64, true},
#endif
#if BENCHMARK_WITH_BOOST
		{"Clipper"     , ClipperWrapper ::BenchmarkUnion    , true},
#endif
#if BENCHMARK_WITH_GEOS
		{"Geos"        , GeosWrapper    ::BenchmarkUnion    , true},
#endif
	};

	auto W = std::setw(16);
	std::cout << W << "Test" << W << "Vertices";
	for(Benchmark &benchmark : benchmarks) {
		std::cout << W << benchmark.m_name;
	}
	std::cout << std::endl;

	std::vector<uint32_t> tests = {1, 2, 3, 5, 7, 10, 15, 22, 33, 47, 68, 100, 150, 220, 330, 470, 680, 1000};
	for(size_t tnum = 0; tnum < tests.size(); ++tnum) {

		std::cout << W << tests[tnum];
		std::cout.flush();

		TestGenerators::Polygon inputs[2];
		TestGenerators::DualGrid(0, TestGenerators::DUALGRID_DEFAULT, tests[tnum], 20.0, false, inputs);

		std::cout << W << (inputs[0].vertices.size() + inputs[1].vertices.size());
		std::cout.flush();

		for(Benchmark &benchmark : benchmarks) {
			TestGenerators::Polygon result;
			double time = 0.0;
			if(benchmark.m_run) {
				size_t loops = 1;
				for( ; ; ) {
					time = benchmark.m_func(inputs[0], inputs[1], result, loops);
					if(time * double(loops) > 1.0)
						break;
					loops = std::max(loops + 1, size_t(1.5 / time));
				}
				benchmark.m_run = (time < 10.0);
			}
			std::cout << W << time;
			std::cout.flush();
		}

		std::cout << std::endl;

	}
	std::cout << "Done." << std::endl;

	return 0;
}
