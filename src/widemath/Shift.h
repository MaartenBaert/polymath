#pragma once

#include "Common.h"

namespace WideMath {

inline uint64_t ShiftLeft_128_64(uint64_t x0, uint64_t x1, uint32_t s) {
	// left shift, return top half
	// r = (x << s) >> 64, s = 0..63
	assert(s < 64);
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	uint64_t r;
	asm(
		"shldq %3, %1, %0"
		: "=rm" (r)
		: "r" (x0), "0" (x1), "Jc" (uint8_t(s))
		: "cc"
	);
	return r;
#else
	return (s == 0)? x1 : (x1 << s) | (x0 >> (64 - s));
#endif
}

inline int64_t ShiftLeft_128_64(uint64_t x0, int64_t x1, uint32_t s) {
	return int64_t(ShiftLeft_128_64(x0, uint64_t(x1), s));
}

inline void ShiftLeft_128_128(uint64_t x0, uint64_t x1, uint32_t s, uint64_t &r0, uint64_t &r1) {
	r1 = ShiftLeft_128_64(x0, x1, s);
	r0 = x0 << s;
}

inline void ShiftLeft_128_128(uint64_t x0, int64_t x1, uint32_t s, uint64_t &r0, int64_t &r1) {
	r1 = ShiftLeft_128_64(x0, x1, s);
	r0 = x0 << s;
}

inline void ShiftLeft_192_192(uint64_t x0, uint64_t x1, uint64_t x2, uint32_t s, uint64_t &r0, uint64_t &r1, uint64_t &r2) {
	r2 = ShiftLeft_128_64(x1, x2, s);
	r1 = ShiftLeft_128_64(x0, x1, s);
	r0 = x0 << s;
}

inline void ShiftLeft_192_192(uint64_t x0, uint64_t x1, int64_t x2, uint32_t s, uint64_t &r0, uint64_t &r1, int64_t &r2) {
	r2 = ShiftLeft_128_64(x1, x2, s);
	r1 = ShiftLeft_128_64(x0, x1, s);
	r0 = x0 << s;
}

inline void ShiftLeft_256_256(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3, uint32_t s, uint64_t &r0, uint64_t &r1, uint64_t &r2, uint64_t &r3) {
	r3 = ShiftLeft_128_64(x2, x3, s);
	r2 = ShiftLeft_128_64(x1, x2, s);
	r1 = ShiftLeft_128_64(x0, x1, s);
	r0 = x0 << s;
}

inline void ShiftLeft_256_256(uint64_t x0, uint64_t x1, uint64_t x2, int64_t x3, uint32_t s, uint64_t &r0, uint64_t &r1, uint64_t &r2, int64_t &r3) {
	r3 = ShiftLeft_128_64(x2, x3, s);
	r2 = ShiftLeft_128_64(x1, x2, s);
	r1 = ShiftLeft_128_64(x0, x1, s);
	r0 = x0 << s;
}

inline uint64_t ShiftRight_128_64(uint64_t x0, uint64_t x1, uint32_t s) {
	// right shift, return bottom half (unsigned)
	// r = x >> s, s = 0..63
	assert(s < 64);
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	uint64_t r;
	asm(
		"shrdq %3, %2, %0"
		: "=rm" (r)
		: "0" (x0), "r" (x1), "Jc" (uint8_t(s))
		: "cc"
	);
	return r;
#else
	return (s == 0)? x0 : (x0 >> s) | (x1 << (64 - s));
#endif
}

inline int64_t ShiftRight_128_64(uint64_t x0, int64_t x1, uint32_t s) {
	return int64_t(ShiftRight_128_64(x0, uint64_t(x1), s));
}

inline void ShiftRight_128_128(uint64_t x0, uint64_t x1, uint32_t s, uint64_t &r0, uint64_t &r1) {
	r0 = ShiftRight_128_64(x0, x1, s);
	r1 = x1 >> s;
}

inline void ShiftRight_128_128(uint64_t x0, int64_t x1, uint32_t s, uint64_t &r0, int64_t &r1) {
	r0 = ShiftRight_128_64(x0, x1, s);
	r1 = x1 >> s;
}

inline void ShiftRight_192_192(uint64_t x0, uint64_t x1, uint64_t x2, uint32_t s, uint64_t &r0, uint64_t &r1, uint64_t &r2) {
	r0 = ShiftRight_128_64(x0, x1, s);
	r1 = ShiftRight_128_64(x1, x2, s);
	r2 = x2 >> s;
}

inline void ShiftRight_192_192(uint64_t x0, uint64_t x1, int64_t x2, uint32_t s, uint64_t &r0, uint64_t &r1, int64_t &r2) {
	r0 = ShiftRight_128_64(x0, x1, s);
	r1 = ShiftRight_128_64(x1, x2, s);
	r2 = x2 >> s;
}

inline void ShiftRight_256_256(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3, uint32_t s, uint64_t &r0, uint64_t &r1, uint64_t &r2, uint64_t &r3) {
	r0 = ShiftRight_128_64(x0, x1, s);
	r1 = ShiftRight_128_64(x1, x2, s);
	r2 = ShiftRight_128_64(x2, x3, s);
	r3 = x3 >> s;
}

inline void ShiftRight_256_256(uint64_t x0, uint64_t x1, uint64_t x2, int64_t x3, uint32_t s, uint64_t &r0, uint64_t &r1, uint64_t &r2, int64_t &r3) {
	r0 = ShiftRight_128_64(x0, x1, s);
	r1 = ShiftRight_128_64(x1, x2, s);
	r2 = ShiftRight_128_64(x2, x3, s);
	r3 = x3 >> s;
}

}
