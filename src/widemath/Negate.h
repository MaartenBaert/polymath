#pragma once

#include "Common.h"

namespace WideMath {

inline void Negate_128(uint64_t x0, uint64_t x1, uint64_t &r0, uint64_t &r1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"negq %0 \n\t"
		"sbbq %3, %1"
		: "=&r" (r0), "=r" (r1)
		: "0" (x0), "erm" (x1), "1" (uint64_t(0))
		: "cc"
	);
#else
	r0 = -x0;
	r1 = (x1 ^ UINT64_MAX) + (r0 == 0);
#endif
}

inline void Negate_128(uint64_t x0, int64_t x1, uint64_t &r0, int64_t &r1) {
	uint64_t ur1;
	Negate_128(x0, uint64_t(x1), r0, ur1);
	r1 = int64_t(ur1);
}

}
