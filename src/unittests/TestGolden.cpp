// Regression tests: the output of every output policy, for every winding rule and integer type, is compared with
// golden hashes of known-good output (see TestCommon.h for the input set and the canonical form). This guards
// refactoring of the output policies against any change in the generated output.
//
// To regenerate the golden data after an intentional change in the output, run:
//     polymath-unittests "Generate golden data" > ../src/unittests/GoldenData.inc
// and check the reason for every changed entry before committing.

#include "TestCommon.h"

#include "3rdparty/catch.hpp"

#include <cinttypes>
#include <cstdio>
#include <map>

using namespace TestCommon;

namespace {

struct GoldenEntry {
	const char *m_input;
	const char *m_output;
	uint64_t m_hash;
};

const GoldenEntry GOLDEN_DATA[] = {
#include "GoldenData.inc"
};

template<typename T>
void HashOutputs(Hasher &hasher, const TestGenerators::Polygon &input, OutputType output_type) {
	PolyMath::Polygon<T> converted = TestGenerators::TypeConverter<T>::ConvertPolygonToType(input);
	for(PolyMath::WindingRule rule : ALL_WINDING_RULES) {
		HashCanonical(hasher, Run(converted, output_type, rule));
	}
}

// Returns the hash for every (input, output) pair, combining all winding rules and integer types.
std::map<std::pair<std::string, std::string>, uint64_t> ComputeHashes() {
	std::map<std::pair<std::string, std::string>, uint64_t> hashes;
	std::vector<TestInput> inputs_i32 = GoldenInputs<int32_t>(), inputs_i64 = GoldenInputs<int64_t>();
	REQUIRE(inputs_i32.size() == inputs_i64.size());
	for(size_t i = 0; i < inputs_i32.size(); ++i) {
		REQUIRE(inputs_i32[i].m_name == inputs_i64[i].m_name);
		for(OutputType output_type : ALL_OUTPUT_TYPES) {
			Hasher hasher;
			HashOutputs<int32_t>(hasher, inputs_i32[i].m_polygon, output_type);
			HashOutputs<int64_t>(hasher, inputs_i64[i].m_polygon, output_type);
			hashes[{inputs_i32[i].m_name, OutputTypeName(output_type)}] = hasher.Get();
		}
	}
	return hashes;
}

}

TEST_CASE("Golden output", "[golden]") {
	std::map<std::pair<std::string, std::string>, uint64_t> hashes = ComputeHashes();
	REQUIRE(sizeof(GOLDEN_DATA) / sizeof(GOLDEN_DATA[0]) == hashes.size());
	for(const GoldenEntry &entry : GOLDEN_DATA) {
		auto it = hashes.find({entry.m_input, entry.m_output});
		INFO("input: " << entry.m_input << ", output: " << entry.m_output);
		REQUIRE(it != hashes.end());
		CHECK(it->second == entry.m_hash);
	}
}

// Hidden test (only runs when selected by name), prints the golden data.
TEST_CASE("Generate golden data", "[.]") {
	for(auto &entry : ComputeHashes()) {
		std::printf("{\"%s\", \"%s\", UINT64_C(0x%016" PRIx64 ")},\n", entry.first.first.c_str(), entry.first.second.c_str(), entry.second);
	}
}
