#pragma once

#include "Common.h"

namespace WideMath {

inline void Add_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t &r0, uint64_t &r1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"addq %4, %0 \n\t"
		"adcq %5, %1"
		: "=&r" (r0), "=r" (r1)
		: "0" (a0), "1" (a1), "erm" (b0), "erm" (b1)
		: "cc"
	);
#else
	r0 = a0 + b0;
	r1 = a1 + b1 + (r0 < a0);
#endif
}

inline void Add_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1, uint64_t &r0, int64_t &r1) {
	uint64_t ur1;
	Add_128(a0, uint64_t(a1), b0, uint64_t(b1), r0, ur1);
	r1 = int64_t(ur1);
}

inline void Add_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t &r0, uint64_t &r1, uint64_t &r2) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"addq %6, %0 \n\t"
		"adcq %7, %1 \n\t"
		"adcq %8, %2"
		: "=&r" (r0), "=&r" (r1), "=r" (r2)
		: "0" (a0), "1" (a1), "2" (a2), "erm" (b0), "erm" (b1), "erm" (b2)
		: "cc"
	);
#else
	r0 = a0 + b0;
	r1 = a1 + b1 + (r0 < a0);
	r2 = a2 + b2 + (r1 < a1 || (r1 == a1 && r0 < a0));
#endif
}

inline void Add_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2, uint64_t &r0, uint64_t &r1, int64_t &r2) {
	uint64_t ur2;
	Add_192(a0, a1, uint64_t(a2), b0, b1, uint64_t(b2), r0, r1, ur2);
	r2 = int64_t(ur2);
}

inline void Add_256(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t b3, uint64_t &r0, uint64_t &r1, uint64_t &r2, uint64_t &r3) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"addq %8, %0 \n\t"
		"adcq %9, %1 \n\t"
		"adcq %10, %2 \n\t"
		"adcq %11, %3"
		: "=&r" (r0), "=&r" (r1), "=&r" (r2), "=r" (r3)
		: "0" (a0), "1" (a1), "2" (a2), "3" (a3), "erm" (b0), "erm" (b1), "erm" (b2), "erm" (b3)
		: "cc"
	);
#else
	bool c;
	r0 = a0 + b0    ; c = (r0 < a0);
	r1 = a1 + b1 + c; c = (r1 < a1 || (r1 == a1 && c));
	r2 = a2 + b2 + c; c = (r2 < a2 || (r2 == a2 && c));
	r3 = a3 + b3 + c;
#endif
}

inline void Add_256(uint64_t a0, uint64_t a1, uint64_t a2, int64_t a3, uint64_t b0, uint64_t b1, uint64_t b2, int64_t b3, uint64_t &r0, uint64_t &r1, uint64_t &r2, int64_t &r3) {
	uint64_t ur3;
	Add_256(a0, a1, a2, uint64_t(a3), b0, b1, b2, uint64_t(b3), r0, r1, r2, ur3);
	r3 = int64_t(ur3);
}

inline bool AddOverflow_64(uint64_t a, uint64_t b, uint64_t &r) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool c;
	asm(
		"addq %3, %1"
		ASM_FLAG_SET("c", "%0")
		: ASM_FLAG_OUT("c", c), "=&r" (r)
		: "1" (a), "erm" (b)
		: "cc"
	);
	return c;
#else
	r = a + b;
	return (r < a);
#endif
}

inline bool AddOverflow_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t &r0, uint64_t &r1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool c;
	asm(
		"addq %5, %1 \n\t"
		"adcq %6, %2"
		ASM_FLAG_SET("c", "%0")
		: ASM_FLAG_OUT("c", c), "=&r" (r0), "=&r" (r1)
		: "1" (a0), "2" (a1), "erm" (b0), "erm" (b1)
		: "cc"
	);
	return c;
#else
	bool c;
	r0 = a0 + b0    ; c = (r0 < a0);
	r1 = a1 + b1 + c; c = (r1 < a1 || (r1 == a1 && c));
	return c;
#endif
}

inline bool AddOverflow_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t &r0, uint64_t &r1, uint64_t &r2) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool c;
	asm(
		"addq %7, %1 \n\t"
		"adcq %8, %2 \n\t"
		"adcq %9, %3"
		ASM_FLAG_SET("c", "%0")
		: ASM_FLAG_OUT("c", c), "=&r" (r0), "=&r" (r1), "=&r" (r2)
		: "1" (a0), "2" (a1), "3" (a2), "erm" (b0), "erm" (b1), "erm" (b2)
		: "cc"
	);
	return c;
#else
	bool c;
	r0 = a0 + b0    ; c = (r0 < a0);
	r1 = a1 + b1 + c; c = (r1 < a1 || (r1 == a1 && c));
	r2 = a2 + b2 + c; c = (r2 < a2 || (r2 == a2 && c));
	return c;
#endif
}

inline bool AddOverflow_256(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t b3, uint64_t &r0, uint64_t &r1, uint64_t &r2, uint64_t &r3) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool c;
	asm(
		"addq %9, %1 \n\t"
		"adcq %10, %2 \n\t"
		"adcq %11, %3 \n\t"
		"adcq %12, %4"
		ASM_FLAG_SET("c", "%0")
		: ASM_FLAG_OUT("c", c), "=&r" (r0), "=&r" (r1), "=&r" (r2), "=&r" (r3)
		: "1" (a0), "2" (a1), "3" (a2), "4" (a3), "erm" (b0), "erm" (b1), "erm" (b2), "erm" (b3)
		: "cc"
	);
	return c;
#else
	bool c;
	r0 = a0 + b0    ; c = (r0 < a0);
	r1 = a1 + b1 + c; c = (r1 < a1 || (r1 == a1 && c));
	r2 = a2 + b2 + c; c = (r2 < a2 || (r2 == a2 && c));
	r3 = a3 + b3 + c; c = (r3 < a3 || (r3 == a3 && c));
	return c;
#endif
}

}
