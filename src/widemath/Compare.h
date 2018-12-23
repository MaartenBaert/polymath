#pragma once

#include "Common.h"

namespace WideMath {

inline bool CompareEqual_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
	return (a0 == b0 && a1 == b1);
}

inline bool CompareEqual_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1) {
	return (a0 == b0 && a1 == b1);
}

inline bool CompareEqual_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2) {
	return (a0 == b0 && a1 == b1 && a2 == b2);
}

inline bool CompareEqual_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2) {
	return (a0 == b0 && a1 == b1 && a2 == b2);
}

inline bool CompareNotEqual_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
	return !CompareEqual_128(a0, a1, b0, b1);
}

inline bool CompareNotEqual_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1) {
	return !CompareEqual_128(a0, a1, b0, b1);
}

inline bool CompareNotEqual_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2) {
	return !CompareEqual_192(a0, a1, a2, b0, b1, b2);
}

inline bool CompareNotEqual_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2) {
	return !CompareEqual_192(a0, a1, a2, b0, b1, b2);
}

inline bool CompareLess_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %4, %2 \n\t"
		"sbbq %5, %3"
		ASM_FLAG_SET("b", "%0")
		: ASM_FLAG_OUT("b", r), "=&r" (a1)
		: "r" (a0), "1" (a1), "erm" (b0), "erm" (b1)
		: "cc"
	);
	return r;
#else
	return (a1 < b1 || (a1 == b1 && a0 < b0));
#endif
}

inline bool CompareLess_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %4, %2 \n\t"
		"sbbq %5, %3"
		ASM_FLAG_SET("l", "%0")
		: ASM_FLAG_OUT("l", r), "=&r" (a1)
		: "r" (a0), "1" (a1), "erm" (b0), "erm" (b1)
		: "cc"
	);
	return r;
#else
	return (a1 < b1 || (a1 == b1 && a0 < b0));
#endif
}

inline bool CompareLess_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %6, %3 \n\t"
		"sbbq %7, %4 \n\t"
		"sbbq %8, %5"
		ASM_FLAG_SET("b", "%0")
		: ASM_FLAG_OUT("b", r), "=&r" (a1), "=&r" (a2)
		: "r" (a0), "1" (a1), "2" (a2), "erm" (b0), "erm" (b1), "erm" (b2)
		: "cc"
	);
	return r;
#else
	return (a2 < b2 || (a2 == b2 && (a1 < b1 || (a1 == b1 && a0 < b0))));
#endif
}

inline bool CompareLess_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %6, %3 \n\t"
		"sbbq %7, %4 \n\t"
		"sbbq %8, %5"
		ASM_FLAG_SET("l", "%0")
		: ASM_FLAG_OUT("l", r), "=&r" (a1), "=&r" (a2)
		: "r" (a0), "1" (a1), "2" (a2), "erm" (b0), "erm" (b1), "erm" (b2)
		: "cc"
	);
	return r;
#else
	return (a2 < b2 || (a2 == b2 && (a1 < b1 || (a1 == b1 && a0 < b0))));
#endif
}

inline bool CompareGreaterEqual_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %4, %2 \n\t"
		"sbbq %5, %3"
		ASM_FLAG_SET("ae", "%0")
		: ASM_FLAG_OUT("ae", r), "=&r" (a1)
		: "r" (a0), "1" (a1), "erm" (b0), "erm" (b1)
		: "cc"
	);
	return r;
#else
	return (a1 > b1 || (a1 == b1 && a0 >= b0));
#endif
}

inline bool CompareGreaterEqual_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %4, %2 \n\t"
		"sbbq %5, %3"
		ASM_FLAG_SET("ge", "%0")
		: ASM_FLAG_OUT("ge", r), "=&r" (a1)
		: "r" (a0), "1" (a1), "erm" (b0), "erm" (b1)
		: "cc"
	);
	return r;
#else
	return (a1 > b1 || (a1 == b1 && a0 >= b0));
#endif
}

inline bool CompareGreaterEqual_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %6, %3 \n\t"
		"sbbq %7, %4 \n\t"
		"sbbq %8, %5"
		ASM_FLAG_SET("ae", "%0")
		: ASM_FLAG_OUT("ae", r), "=&r" (a1), "=&r" (a2)
		: "r" (a0), "1" (a1), "2" (a2), "erm" (b0), "erm" (b1), "erm" (b2)
		: "cc"
	);
	return r;
#else
	return (a2 > b2 || (a2 == b2 && (a1 > b1 || (a1 == b1 && a0 >= b0))));
#endif
}

inline bool CompareGreaterEqual_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2) {
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	bool r;
	asm(
		"cmpq %6, %3 \n\t"
		"sbbq %7, %4 \n\t"
		"sbbq %8, %5"
		ASM_FLAG_SET("ge", "%0")
		: ASM_FLAG_OUT("ge", r), "=&r" (a1), "=&r" (a2)
		: "r" (a0), "1" (a1), "2" (a2), "erm" (b0), "erm" (b1), "erm" (b2)
		: "cc"
	);
	return r;
#else
	return (a2 > b2 || (a2 == b2 && (a1 > b1 || (a1 == b1 && a0 >= b0))));
#endif
}

inline bool CompareGreater_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
	return CompareLess_128(b0, b1, a0, a1);
}

inline bool CompareGreater_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1) {
	return CompareLess_128(b0, b1, a0, a1);
}

inline bool CompareGreater_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2) {
	return CompareLess_192(b0, b1, b2, a0, a1, a2);
}

inline bool CompareGreater_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2) {
	return CompareLess_192(b0, b1, b2, a0, a1, a2);
}

inline bool CompareLessEqual_128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1) {
	return CompareGreaterEqual_128(b0, b1, a0, a1);
}

inline bool CompareLessEqual_128(uint64_t a0, int64_t a1, uint64_t b0, int64_t b1) {
	return CompareGreaterEqual_128(b0, b1, a0, a1);
}

inline bool CompareLessEqual_192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2) {
	return CompareGreaterEqual_192(b0, b1, b2, a0, a1, a2);
}

inline bool CompareLessEqual_192(uint64_t a0, uint64_t a1, int64_t a2, uint64_t b0, uint64_t b1, int64_t b2) {
	return CompareGreaterEqual_192(b0, b1, b2, a0, a1, a2);
}

}
