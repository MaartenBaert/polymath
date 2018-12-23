#pragma once

#include "Common.h"

#include "Add.h"

namespace WideMath {

inline void Multiply_64x64_128(uint64_t a, uint64_t b, uint64_t &m0, uint64_t &m1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"mulq %3"
		: "=a" (m0), "=d" (m1)
		: "0" (a), "rm" (b)
		: "cc"
	);
#else
	uint32_t aa0 = uint32_t(a), aa1 = uint32_t(a >> 32);
	uint32_t bb0 = uint32_t(b), bb1 = uint32_t(b >> 32);
	uint64_t mm10 = uint64_t(aa0) * uint64_t(bb0);
	uint64_t mm21a = uint64_t(aa0) * uint64_t(bb1);
	uint64_t mm21b = uint64_t(aa1) * uint64_t(bb0);
	uint64_t mm32 = uint64_t(aa1) * uint64_t(bb1);
	m0 = mm10 + (mm21a << 32) + (mm21b << 32);
	uint64_t carry = (mm10 >> 32) + (mm21a & uint64_t(UINT32_MAX)) + (mm21b & uint64_t(UINT32_MAX));
	m1 = mm32 + (mm21a >> 32) + (mm21b >> 32) + (carry >> 32);
#endif
}

inline void Multiply_64x64_128(int64_t a, int64_t b, uint64_t &m0, int64_t &m1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"imulq %3"
		: "=a" (m0), "=d" (m1)
		: "0" (a), "rm" (b)
		: "cc"
	);
#else
	uint64_t um1;
	Multiply_64x64_128(uint64_t(a), uint64_t(b), m0, um1);
	um1 -= uint64_t(b & (a >> 63));
	um1 -= uint64_t(a & (b >> 63));
	m1 = int64_t(um1);
#endif
}

inline void Multiply_64x64_128(uint64_t a, int64_t b, uint64_t &m0, int64_t &m1) {
	uint64_t um1;
	Multiply_64x64_128(a, uint64_t(b), m0, um1);
	um1 -= a & uint64_t(b >> 63);
	m1 = int64_t(um1);
}

inline void Multiply_64x64_128(int64_t a, uint64_t b, uint64_t &m0, int64_t &m1) {
	uint64_t um1;
	Multiply_64x64_128(uint64_t(a), b, m0, um1);
	um1 -= b & uint64_t(a >> 63);
	m1 = int64_t(um1);
}

inline void Multiply_128x64_192(uint64_t a0, uint64_t a1, uint64_t b, uint64_t &m0, uint64_t &m1, uint64_t &m2) {
	uint64_t t01, t10, t11;
	Multiply_64x64_128(a0, b, m0, t01);
	Multiply_64x64_128(a1, b, t10, t11);
	Add_128(t10, t11, t01, 0, m1, m2);
}

inline void Multiply_128x64_192(uint64_t a0, uint64_t a1, int64_t b, uint64_t &m0, uint64_t &m1, int64_t &m2) {
	uint64_t t10;
	int64_t t01, t11;
	Multiply_64x64_128(a0, b, m0, t01);
	Multiply_64x64_128(a1, b, t10, t11);
	Add_128(t10, t11, uint64_t(t01), t01 >> 63, m1, m2);
}

inline void Multiply_128x64_192(uint64_t a0, int64_t a1, uint64_t b, uint64_t &m0, uint64_t &m1, int64_t &m2) {
	uint64_t t01, t10;
	int64_t t11;
	Multiply_64x64_128(a0, b, m0, t01);
	Multiply_64x64_128(a1, b, t10, t11);
	Add_128(t10, t11, t01, 0, m1, m2);
}

inline void Multiply_128x64_192(uint64_t a0, int64_t a1, int64_t b, uint64_t &m0, uint64_t &m1, int64_t &m2) {
	uint64_t t10;
	int64_t t01, t11;
	Multiply_64x64_128(a0, b, m0, t01);
	Multiply_64x64_128(a1, b, t10, t11);
	Add_128(t10, t11, uint64_t(t01), t01 >> 63, m1, m2);
}

inline void Multiply_128x128_256(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t &m0, uint64_t &m1, uint64_t &m2, uint64_t &m3) {
	uint64_t t001, t010, t011, t100, t101, t110, t111;
	Multiply_64x64_128(a0, b0, m0, t001);
	Multiply_64x64_128(a0, b1, t010, t011);
	Multiply_64x64_128(a1, b0, t100, t101);
	Multiply_64x64_128(a1, b1, t110, t111);
	uint64_t u1, u2, u3;
	Add_192(t010, t110, t111, t100, t101, 0, u1, u2, u3);
	Add_192(u1, u2, u3, t001, t011, 0, m1, m2, m3);
}

inline void Multiply_128x128_256(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1, uint64_t &m0, uint64_t &m1, uint64_t &m2, int64_t &m3) {
	uint64_t t001, t010, t100, t110;
	int64_t t011, t101, t111;
	Multiply_64x64_128(a0, b0, m0, t001);
	Multiply_64x64_128(a0, b1, t010, t011);
	Multiply_64x64_128(a1, b0, t100, t101);
	Multiply_64x64_128(a1, b1, t110, t111);
	uint64_t u1, u2;
	int64_t u3;
	Add_192(t010, t110, t111, t100, t101, 0, u1, u2, u3);
	Add_192(u1, u2, u3, t001, t011, 0, m1, m2, m3);
}

}
