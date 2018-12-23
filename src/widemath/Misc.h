#pragma once

#include "Common.h"

#include "Subtract.h"

namespace WideMath {

inline uint32_t Abs_32(int32_t x) {
	uint32_t s = uint32_t(x >> 31);
	return (uint32_t(x) ^ s) - s;
}

inline uint64_t Abs_64(int64_t x) {
	uint64_t s = uint64_t(x >> 63);
	return (uint64_t(x) ^ s) - s;
}

inline void Abs_128(uint64_t x0, int64_t x1, uint64_t &r0, uint64_t &r1) {
	uint64_t s = uint64_t(x1 >> 63);
	Subtract_128(x0 ^ s, uint64_t(x1) ^ s, s, s, r0, r1);
}

inline uint32_t BitScanReverse_32(uint32_t x) {
	// the result is undefined when the input is zero
	assert(x != 0);
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	uint32_t r;
	asm(
		"bsrl %1, %0"
		: "=r" (r)
		: "rm" (x)
		: "cc"
	);
	return r;
#else
	uint32_t r = 0;
	if(x >> 16) { x >>= 16; r |= 16; }
	if(x >>  8) { x >>=  8; r |=  8; }
	if(x >>  4) { x >>=  4; r |=  4; }
	if(x >>  2) { x >>=  2; r |=  2; }
	if(x >>  1) { x >>=  1; r |=  1; }
	return r;
#endif
}

inline uint32_t BitScanReverse_64(uint64_t x) {
	// the result is undefined when the input is zero
	assert(x != 0);
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	uint64_t r;
	asm(
		"bsrq %1, %0"
		: "=r" (r)
		: "rm" (x)
		: "cc"
	);
	return uint32_t(r);
#else
	uint32_t r = 0;
	if(x >> 32) { x >>= 32; r |= 32; }
	if(x >> 16) { x >>= 16; r |= 16; }
	if(x >>  8) { x >>=  8; r |=  8; }
	if(x >>  4) { x >>=  4; r |=  4; }
	if(x >>  2) { x >>=  2; r |=  2; }
	if(x >>  1) { x >>=  1; r |=  1; }
	return r;
#endif
}

}
