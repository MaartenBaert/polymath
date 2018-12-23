/*
Copyright (C) 2016  The AlterPCB team
Contact: Maarten Baert <maarten-baert@hotmail.com>

This file is part of AlterPCB.

AlterPCB is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

AlterPCB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this AlterPCB.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "widemath/WideMath.h"

#include "3rdparty/catch.hpp"

#include <iostream>
#include <random>

constexpr uint64_t RANDOM_SEED = UINT64_C(0x9888492a624ba624);
constexpr uint32_t NUM_TESTS_RANDOM = 1000000;

uint64_t RandomU64(std::mt19937_64 &rng) {
	uint64_t val = rng() >> (rng() & 63);
	val ^= -(rng() & 1);
	return val;
}

int64_t RandomS64(std::mt19937_64 &rng) {
	uint64_t val = rng() >> (rng() & 63);
	val ^= -(rng() & ((uint64_t(1) << 63) | 1));
	return int64_t(val);
}

TEST_CASE("Wide multiplication (Multiply_64x64_128)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// unsigned
	uint32_t errors1 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint64_t a = RandomU64(rng), b = RandomU64(rng);
		uint64_t m0, m1;
		WideMath::Multiply_64x64_128(a, b, m0, m1);
		uint64_t r0 = 0, r1 = 0, t0 = a, t1 = 0;
		for(uint32_t i = 0; i < 64; ++i) {
			if((b >> i) & 1) {
				WideMath::Add_128(r0, r1, t0, t1, r0, r1);
			}
			WideMath::ShiftLeft_128_128(t0, t1, 1, t0, t1);
		}
		errors1 += (m0 != r0 || m1 != r1);
	}
	REQUIRE(errors1 == 0);

	// signed
	uint32_t errors2 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		int64_t a = RandomS64(rng), b = RandomS64(rng);
		uint64_t m0;
		int64_t m1;
		WideMath::Multiply_64x64_128(a, b, m0, m1);
		uint64_t r0 = 0, t0 = uint64_t(a);
		int64_t r1 = 0, t1 = a >> 63;
		for(uint32_t i = 0; i < 64; ++i) {
			if((b >> i) & 1) {
				if(i == 63) {
					WideMath::Subtract_128(r0, r1, t0, t1, r0, r1);
				} else {
					WideMath::Add_128(r0, r1, t0, t1, r0, r1);
				}
			}
			WideMath::ShiftLeft_128_128(t0, t1, 1, t0, t1);
		}
		errors2 += (m0 != r0 || m1 != r1);
	}
	REQUIRE(errors2 == 0);

}

TEST_CASE("Wide division (DivideFloor_128x64_64)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// unsigned
	uint32_t errors1 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint64_t a = RandomU64(rng);
		uint64_t b = RandomU64(rng);
		if(b == 0)
			b = 1;
		uint64_t m0, m1;
		WideMath::Multiply_64x64_128(a, b, m0, m1);
		uint64_t of0, of1;
		WideMath::Multiply_64x64_128(b, RandomU64(rng), of0, of1);
		WideMath::Add_128(m0, m1, of1, 0, m0, m1);
		uint64_t q, r;
		WideMath::DivideFloor_128x64_64(m0, m1, b, q, r);
		errors1 += (q != a || r != of1);
	}
	REQUIRE(errors1 == 0);

	// signed
	uint32_t errors2 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		int64_t a = RandomS64(rng);
		int64_t b = RandomS64(rng);
		if(b == 0)
			b = 1;
		uint64_t m0;
		int64_t m1;
		WideMath::Multiply_64x64_128(a, b, m0, m1);
		uint64_t of0;
		int64_t of1;
		WideMath::Multiply_64x64_128(b, RandomU64(rng), of0, of1);
		WideMath::Add_128(of0, of1, uint64_t(b >> 63), 0, of0, of1);
		WideMath::Add_128(m0, m1, uint64_t(of1), of1 >> 63, m0, m1);
		int64_t q, r;
		WideMath::DivideFloor_128x64_64(m0, m1, b, q, r);
		errors2 += (q != a || r != of1);
	}
	REQUIRE(errors2 == 0);

}

TEST_CASE("Wide division (DivideFloor_128x64_128)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// unsigned
	uint32_t errors1 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint32_t split = rng() & 63;
		uint64_t a0 = RandomU64(rng);
		uint64_t a1 = RandomU64(rng);
		uint64_t b = RandomU64(rng);
		a0 = WideMath::ShiftLeft_128_64(a0, a1, split);
		a1 = WideMath::ShiftLeft_128_64(a1, uint64_t(0), split);
		b >>= split;
		if(b == 0)
			b = 1;
		uint64_t m0, m1;
		WideMath::Multiply_64x64_128(a0, b, m0, m1);
		m1 += a1 * b;
		uint64_t of0, of1;
		WideMath::Multiply_64x64_128(b, RandomU64(rng), of0, of1);
		WideMath::Add_128(m0, m1, of1, 0, m0, m1);
		uint64_t q0, q1, r;
		WideMath::DivideFloor_128x64_128(m0, m1, b, q0, q1, r);
		errors1 += (q0 != a0 || q1 != a1 || r != of1);
	}
	REQUIRE(errors1 == 0);

	// signed
	uint32_t errors2 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint32_t split = rng() & 63;
		uint64_t a0 = RandomU64(rng);
		int64_t a1 = RandomS64(rng);
		int64_t b = RandomS64(rng);
		a0 = WideMath::ShiftLeft_128_64(a0, uint64_t(a1), split);
		a1 = WideMath::ShiftLeft_128_64(uint64_t(a1), a1 >> 63, split);
		b >>= split;
		if(b == 0)
			b = 1;
		uint64_t m0;
		int64_t m1;
		WideMath::Multiply_64x64_128(a0, b, m0, m1);
		m1 = int64_t(uint64_t(m1) + uint64_t(a1) * uint64_t(b));
		uint64_t of0;
		int64_t of1;
		WideMath::Multiply_64x64_128(b, RandomU64(rng), of0, of1);
		WideMath::Add_128(of0, of1, uint64_t(b >> 63), 0, of0, of1);
		WideMath::Add_128(m0, m1, uint64_t(of1), of1 >> 63, m0, m1);
		uint64_t q0;
		int64_t q1, r;
		WideMath::DivideFloor_128x64_128(m0, m1, b, q0, q1, r);
		errors2 += (q0 != a0 || q1 != a1 || r != of1);
	}
	REQUIRE(errors2 == 0);

}

TEST_CASE("Wide division (DivideRound_128x64_128)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// signed
	uint32_t errors2 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint32_t split = rng() & 63;
		uint64_t a0 = RandomU64(rng);
		int64_t a1 = RandomS64(rng);
		int64_t b = RandomS64(rng);
		a0 = WideMath::ShiftLeft_128_64(a0, uint64_t(a1), split);
		a1 = WideMath::ShiftLeft_128_64(uint64_t(a1), a1 >> 63, split);
		b >>= split;
		if(b == 0)
			b = 1;
		uint64_t m0;
		int64_t m1;
		WideMath::Multiply_64x64_128(a0, b, m0, m1);
		m1 = int64_t(uint64_t(m1) + uint64_t(a1) * uint64_t(b));
		uint64_t of0;
		int64_t of1;
		WideMath::Multiply_64x64_128(b, RandomU64(rng), of0, of1);
		WideMath::Add_128(of0, of1, uint64_t(b >> 63), 0, of0, of1);
		of1 -= (b - (b >> 63)) >> 1;
		WideMath::Add_128(m0, m1, uint64_t(of1), of1 >> 63, m0, m1);
		uint64_t q0;
		int64_t q1, r;
		WideMath::DivideRound_128x64_128(m0, m1, b, q0, q1, r);
		errors2 += (q0 != a0 || q1 != a1 || r != of1);
		/*if(q0 != a0 || q1 != a1 || r != of1) {
			std::cerr << "Error: (" << a1 << "," << a0 << ") * " << b << " + " << of1 << " = (" << m1 << "," << m0 << ")" << std::endl;
			std::cerr << "       (" << m1 << "," << m0 << ") / " << b << " = (" << q1 << "," << q0 << ")" << std::endl;
		}*/
	}
	REQUIRE(errors2 == 0);

}

TEST_CASE("Wide division (DivideCeil_128x64_128)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// signed
	uint32_t errors2 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint32_t split = rng() & 63;
		uint64_t a0 = RandomU64(rng);
		int64_t a1 = RandomS64(rng);
		int64_t b = RandomS64(rng);
		a0 = WideMath::ShiftLeft_128_64(a0, uint64_t(a1), split);
		a1 = WideMath::ShiftLeft_128_64(uint64_t(a1), a1 >> 63, split);
		b >>= split;
		if(b == 0)
			b = 1;
		uint64_t m0;
		int64_t m1;
		WideMath::Multiply_64x64_128(a0, b, m0, m1);
		m1 = int64_t(uint64_t(m1) + uint64_t(a1) * uint64_t(b));
		uint64_t of0;
		int64_t of1;
		WideMath::Multiply_64x64_128(b, RandomU64(rng), of0, of1);
		WideMath::Add_128(of0, of1, uint64_t(b >> 63), 0, of0, of1);
		of1 = -of1;
		WideMath::Add_128(m0, m1, uint64_t(of1), of1 >> 63, m0, m1);
		uint64_t q0;
		int64_t q1, r;
		WideMath::DivideCeil_128x64_128(m0, m1, b, q0, q1, r);
		errors2 += (q0 != a0 || q1 != a1 || r != of1);
		/*if(q0 != a0 || q1 != a1 || r != of1) {
			std::cerr << "Error: (" << a1 << "," << a0 << ") * " << b << " + " << of1 << " = (" << m1 << "," << m0 << ")" << std::endl;
			std::cerr << "       (" << m1 << "," << m0 << ") / " << b << " = (" << q1 << "," << q0 << ")" << std::endl;
		}*/
	}
	REQUIRE(errors2 == 0);

}

TEST_CASE("Wide division (DivideFloor_192x128_64)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// unsigned
	uint32_t errors1 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint64_t a = RandomU64(rng);
		uint64_t b0 = RandomU64(rng);
		uint64_t b1 = RandomU64(rng);
		if(b0 == 0 && b1 == 0)
			b0 = 1;
		uint64_t m0, m1, m2;
		WideMath::Multiply_128x64_192(b0, b1, a, m0, m1, m2);
		uint64_t of0, of1, of2, of3;
		WideMath::Multiply_128x128_256(b0, b1, RandomU64(rng), RandomU64(rng), of0, of1, of2, of3);
		WideMath::Add_192(m0, m1, m2, of2, of3, 0, m0, m1, m2);
		uint64_t q, r0, r1;
		WideMath::DivideFloor_192x128_64(m0, m1, m2, b0, b1, q, r0, r1);
		errors1 += (q != a || r0 != of2 || r1 != of3);
	}
	REQUIRE(errors1 == 0);

}

TEST_CASE("Wide division (DivideFloor_256x128_128)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	// unsigned
	uint32_t errors1 = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint64_t a0 = RandomU64(rng);
		uint64_t a1 = RandomU64(rng);
		uint64_t b0 = RandomU64(rng);
		uint64_t b1 = RandomU64(rng);
		if(b0 == 0 && b1 == 0)
			b0 = 1;
		uint64_t m0, m1, m2, m3;
		WideMath::Multiply_128x128_256(a0, a1, b0, b1, m0, m1, m2, m3);
		uint64_t of0, of1, of2, of3;
		WideMath::Multiply_128x128_256(b0, b1, RandomU64(rng), RandomU64(rng), of0, of1, of2, of3);
		WideMath::Add_256(m0, m1, m2, m3, of2, of3, 0, 0, m0, m1, m2, m3);
		uint64_t q0, q1, r0, r1;
		WideMath::DivideFloor_256x128_128(m0, m1, m2, m3, b0, b1, q0, q1, r0, r1);
		errors1 += (q0 != a0 || q1 != a1 || r0 != of2 || r1 != of3);
		/*if(q0 != a0 || q1 != a1 || r0 != of2 || r1 != of3) {
			std::cerr << "Error: (" << a1 << "," << a0 << ") * (" << b1 << "," << b0 << ") + (" << of3 << "," << of2 << ") = (" << m3 << "," << m2 << "," << m1 << "," << m0 << ")" << std::endl;
			std::cerr << "       (" << m3 << "," << m2 << "," << m1 << "," << m0 << ") / (" << b1 << "," << b0 << ") = (" << q1 << "," << q0 << ")" << std::endl;
			std::cerr << "       (" << m3 << "," << m2 << "," << m1 << "," << m0 << ") % (" << b1 << "," << b0 << ") = (" << r1 << "," << r0 << ")" << std::endl;
		}*/
	}
	REQUIRE(errors1 == 0);

}

TEST_CASE("Wide square root (SqrtFloor_128_64)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	size_t errors = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint64_t val = RandomU64(rng);
		uint64_t m0, m1;
		WideMath::Multiply_64x64_128(val, val, m0, m1);
		uint64_t of0, of1;
		WideMath::Multiply_64x64_128(val, RandomU64(rng), of0, of1);
		WideMath::Add_128(of0, of1, uint64_t(1) << 62, 0, of0, of1);
		WideMath::ShiftRight_128_128(of0, of1, 63, of0, of1);
		WideMath::Add_128(m0, m1, of0, of1, m0, m1);
		uint64_t res = WideMath::SqrtFloor_128_64(m0, m1);
		errors += (val != res);
		/*if(val != res) {
			std::cerr << "Error: sqr(" << val << ") + (" << of1 << "," << of0 << ") = (" << m1 << "," << m0 << ")" << std::endl;
			std::cerr << "       sqrt(" << m1 << "," << m0 << ") = " << res << std::endl;
		}*/
	}
	REQUIRE(errors == 0);

}

TEST_CASE("Wide square root (SqrtRound_128_64)", "[widemath]") {

	std::mt19937_64 rng(RANDOM_SEED);

	size_t errors = 0;
	for(uint32_t i = 0; i < NUM_TESTS_RANDOM; ++i) {
		uint64_t val = RandomU64(rng);
		uint64_t m0, m1;
		WideMath::Multiply_64x64_128(val, val, m0, m1);
		uint64_t of0, of1;
		WideMath::Multiply_64x64_128(val, RandomU64(rng), of0, of1);
		WideMath::ShiftRight_128_128(of0, of1, 63, of0, of1);
		WideMath::Subtract_128(of0, of1, val, 0, of0, of1);
		WideMath::Subtract_128(m0, m1, of0, of1, m0, m1);
		uint64_t res = WideMath::SqrtRound_128_64(m0, m1);
		errors += (val != res);
		/*if(val != res) {
			std::cerr << "Error: sqr(" << val << ") + (" << of1 << "," << of0 << ") = (" << m1 << "," << m0 << ")" << std::endl;
			std::cerr << "       sqrt(" << m1 << "," << m0 << ") = " << res << std::endl;
		}*/
	}
	REQUIRE(errors == 0);

}
