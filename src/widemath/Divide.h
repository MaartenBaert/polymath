#pragma once

#include "Common.h"

#include "Add.h"
#include "Compare.h"
#include "Misc.h"
#include "Multiply.h"
#include "Shift.h"
#include "Subtract.h"

namespace WideMath {

// fundamental building blocks

inline void DivideFloor_128x64_64(uint64_t n0, uint64_t n1, uint64_t d, uint64_t &q, uint64_t &r) {
	// division by zero or divisions where the result overflows are forbidden
	// q = floor(n / d), r = n - d * q
	assert(d != 0 && n1 < d);
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"divq %4"
		: "=a" (q), "=d" (r)
		: "0" (n0), "1" (n1), "rm" (d)
		: "cc"
	);
#else

	// based on https://www.codeproject.com/Tips/785014/UInt-Division-Modulus

	uint32_t s = 63 ^ BitScanReverse_64(d);
	d <<= s;
	uint64_t dd1 = d >> 32;
	uint64_t dd0 = d & uint64_t(UINT32_MAX);

	uint64_t nn32 = ShiftLeft_128_64(n0, n1, s), nn10 = n0 << s;

	uint64_t nn1 = nn10 >> 32;
	uint64_t nn0 = nn10 & uint64_t(UINT32_MAX);

	uint64_t qq1 = nn32 / dd1;
	uint64_t rr1 = nn32 % dd1;
	uint64_t mm1 = qq1 * dd0;
	if(mm1 > ((rr1 << 32) | nn1)) {
		--qq1;
		rr1 += dd1;
		if(rr1 <= uint64_t(UINT32_MAX)) {
			mm1 -= dd0;
			if(mm1 > ((rr1 << 32) | nn1)) {
				--qq1;
			}
		}
	}

	uint64_t nn21 = (nn32 << 32) + (nn1 - qq1 * d);

	uint64_t qq0 = nn21 / dd1;
	uint64_t rr0 = nn21 % dd1;
	uint64_t mm0 = qq0 * dd0;
	if(mm0 > ((rr0 << 32) | nn0)) {
		--qq0;
		rr0 += dd1;
		if(rr0 <= uint64_t(UINT32_MAX)) {
			mm0 -= dd0;
			if(mm0 > ((rr0 << 32) | nn0)) {
				--qq0;
			}
		}
	}

	q = (qq1 << 32) | qq0;
	r = ((nn21 << 32) + (nn0 - qq0 * d)) >> s;

#endif
}

inline void DivideTrunc_128x64_64(uint64_t n0, int64_t n1, int64_t d, int64_t &q, int64_t &r) {
	// division by zero or divisions where the result overflows are forbidden
	// q = trunc(n / d), r = n - d * q (r has the same sign as n)
#ifndef NDEBUG
	{
		assert(d != 0);
		uint64_t min0, max0;
		int64_t min1, max1;
		if(d >= 0) {
			Multiply_64x64_128(d, INT64_MIN, min0, min1);
			Multiply_64x64_128(d, INT64_MAX, max0, max1);
		} else {
			Multiply_64x64_128(d, INT64_MAX, min0, min1);
			Multiply_64x64_128(d, INT64_MIN, max0, max1);
		}
		if(n1 >= 0) {
			Add_128(max0, max1, Abs_64(d) - 1, 0, max0, max1);
		} else {
			Subtract_128(min0, min1, Abs_64(d) - 1, 0, min0, min1);
		}
		assert(CompareGreaterEqual_128(n0, n1, min0, min1) && CompareLessEqual_128(n0, n1, max0, max1));
	}
#endif
#if WIDEMATH_USE_ASM && defined(__GNUC__) && defined(__x86_64__)
	asm(
		"idivq %4"
		: "=a" (q), "=d" (r)
		: "0" (n0), "1" (n1), "rm" (d)
		: "cc"
	);
#else

	// extract signs
	uint64_t sn = uint64_t(n1 >> 63);
	uint64_t sd = uint64_t(d >> 63);

	// transform inputs
	uint64_t un0, un1;
	Subtract_128(n0 ^ sn, uint64_t(n1) ^ sn, sn, sn, un0, un1);
	uint64_t ud = (uint64_t(d) ^ sd) - sd;

	// do unsigned division
	uint64_t uq, ur;
	DivideFloor_128x64_64(un0, un1, ud, uq, ur);

	// transform outputs
	uint64_t sq = sn ^ sd;
	q = int64_t((uq ^ sq) - sq);
	r = int64_t((ur ^ sn) - sn);

#endif
}

inline void DivideFloor_128x64_64(uint64_t n0, int64_t n1, int64_t d, int64_t &q, int64_t &r) {
	// division by zero or divisions where the result overflows are forbidden
	// q = floor(n / d), r = n - d * q (r has the same sign as d)
#ifndef NDEBUG
	{
		assert(d != 0);
		uint64_t min0, max0;
		int64_t min1, max1;
		if(d >= 0) {
			Multiply_64x64_128(d, INT64_MIN, min0, min1);
			Multiply_64x64_128(d, INT64_MAX, max0, max1);
			Add_128(max0, max1, Abs_64(d) - 1, 0, max0, max1);
		} else {
			Multiply_64x64_128(d, INT64_MAX, min0, min1);
			Multiply_64x64_128(d, INT64_MIN, max0, max1);
			Subtract_128(min0, min1, Abs_64(d) - 1, 0, min0, min1);
		}
		assert(CompareGreaterEqual_128(n0, n1, min0, min1) && CompareLessEqual_128(n0, n1, max0, max1));
	}
#endif
	uint64_t sn = uint64_t(n1 >> 63);
	uint64_t sd = uint64_t(d >> 63);
	uint64_t offset = (uint64_t(d) & (sn ^ sd)) + sn - sd;
	Subtract_128(n0, n1, offset, int64_t(offset) >> 63, n0, n1);
	DivideTrunc_128x64_64(n0, n1, d, q, r);
	r = int64_t(uint64_t(r) + offset);
}

inline void DivideFloor_192x128_64(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t d0, uint64_t d1, uint64_t &q, uint64_t &r0, uint64_t &r1) {
	// division by zero or divisions where the result overflows are forbidden
	// q = floor(n / d), r = n - d * q
	assert(CompareNotEqual_128(d0, d1, 0, 0) && CompareLess_128(n1, n2, d0, d1));

	if(d1 == 0) {
		assert(n2 == 0);
		r1 = 0;
		DivideFloor_128x64_64(n0, n1, d0, q, r0);
		return;
	}

	uint32_t s = 63 ^ BitScanReverse_64(d1);
	ShiftLeft_128_128(d0, d1, s, d0, d1);
	ShiftLeft_192_192(n0, n1, n2, s, n0, n1, n2);

	uint64_t qq0;
	if(n2 == d1) {
		qq0 = UINT64_MAX;
		n1 -= d0;
		if(!AddOverflow_128(n0, n1, d0, d1, n0, n1)) {
			--qq0;
			Add_128(n0, n1, d0, d1, n0, n1);
		}
	} else {
		uint64_t mm0, mm1;
		DivideFloor_128x64_64(n1, n2, d1, qq0, n1);
		Multiply_64x64_128(d0, qq0, mm0, mm1);
		if(SubtractOverflow_128(n0, n1, mm0, mm1, n0, n1)) {
			--qq0;
			if(!AddOverflow_128(n0, n1, d0, d1, n0, n1)) {
				--qq0;
				Add_128(n0, n1, d0, d1, n0, n1);
			}
		}
	}
	q = qq0;

	ShiftRight_128_128(n0, n1, s, r0, r1);

}

inline void DivideFloor_256x128_128(uint64_t n0, uint64_t n1, uint64_t n2, uint64_t n3, uint64_t d0, uint64_t d1, uint64_t &q0, uint64_t &q1, uint64_t &r0, uint64_t &r1) {
	// division by zero or divisions where the result overflows are forbidden
	// q = floor(n / d), r = n - d * q
	assert(CompareNotEqual_128(d0, d1, 0, 0) && CompareLess_128(n2, n3, d0, d1));

	if(d1 == 0) {
		assert(n3 == 0);
		r1 = 0;
		DivideFloor_128x64_64(n1, n2, d0, q1, n1);
		DivideFloor_128x64_64(n0, n1, d0, q0, r0);
		return;
	}

	uint32_t s = 63 ^ BitScanReverse_64(d1);
	ShiftLeft_128_128(d0, d1, s, d0, d1);
	ShiftLeft_256_256(n0, n1, n2, n3, s, n0, n1, n2, n3);

	uint64_t qq1;
	if(n3 == d1) {
		qq1 = UINT64_MAX;
		n2 -= d0;
		if(!AddOverflow_128(n1, n2, d0, d1, n1, n2)) {
			--qq1;
			Add_128(n1, n2, d0, d1, n1, n2);
		}
	} else {
		uint64_t mm0, mm1;
		DivideFloor_128x64_64(n2, n3, d1, qq1, n2);
		Multiply_64x64_128(d0, qq1, mm0, mm1);
		if(SubtractOverflow_128(n1, n2, mm0, mm1, n1, n2)) {
			--qq1;
			if(!AddOverflow_128(n1, n2, d0, d1, n1, n2)) {
				--qq1;
				Add_128(n1, n2, d0, d1, n1, n2);
			}
		}
	}
	q1 = qq1;

	uint64_t qq0;
	if(n2 == d1) {
		qq0 = UINT64_MAX;
		n1 -= d0;
		if(!AddOverflow_128(n0, n1, d0, d1, n0, n1)) {
			--qq0;
			Add_128(n0, n1, d0, d1, n0, n1);
		}
	} else {
		uint64_t mm0, mm1;
		DivideFloor_128x64_64(n1, n2, d1, qq0, n1);
		Multiply_64x64_128(d0, qq0, mm0, mm1);
		if(SubtractOverflow_128(n0, n1, mm0, mm1, n0, n1)) {
			--qq0;
			if(!AddOverflow_128(n0, n1, d0, d1, n0, n1)) {
				--qq0;
				Add_128(n0, n1, d0, d1, n0, n1);
			}
		}
	}
	q0 = qq0;

	ShiftRight_128_128(n0, n1, s, r0, r1);

}

// derived (unsigned)

inline void DivideFloor_128x64_128(uint64_t n0, uint64_t n1, uint64_t d, uint64_t &q0, uint64_t &q1, uint64_t &r) {
	// division by zero is forbidden
	// q = floor(n / d), r = n - d * q
	assert(d != 0);
	q1 = n1 / d;
	n1 = n1 % d;
	DivideFloor_128x64_64(n0, n1, d, q0, r);
}

// derived (signed)

inline void DivideFloor_128x64_128(uint64_t n0, int64_t n1, int64_t d, uint64_t &q0, int64_t &q1, int64_t &r) {
	// division by zero or divisions where the result overflows are forbidden
	// q = floor(n / d), r = n - d * q (r has the same sign as d)
	assert(d != 0 && (d != -1 || CompareNotEqual_128(n0, n1, 0, INT64_MIN)));

	// extract signs
	uint64_t sn = uint64_t(n1 >> 63);
	uint64_t sd = uint64_t(d >> 63);

	// transform inputs
	uint64_t un0, un1;
	Subtract_128(n0 ^ sn, uint64_t(n1) ^ sn, sn, sn, un0, un1);
	uint64_t ud = (uint64_t(d) ^ sd) - sd;

	// add offset
	uint64_t sq = sn ^ sd;
	uint64_t offset = (ud - 1) & sq;
	Add_128(un0, un1, offset, 0, un0, un1);

	// do unsigned division
	uint64_t uq0, uq1, ur;
	DivideFloor_128x64_128(un0, un1, ud, uq0, uq1, ur);

	// transform outputs
	Subtract_128(uq0 ^ sq, int64_t(uq1 ^ sq), sq, int64_t(sq), q0, q1);
	r = int64_t(((ur - offset) ^ sn) - sn);

}

inline void DivideRound_128x64_128(uint64_t n0, int64_t n1, int64_t d, uint64_t &q0, int64_t &q1, int64_t &r) {
	// division by zero or divisions where the result overflows are forbidden
	// q = round(n / d), r = n - d * q
	assert(d != 0 && (d != -1 || CompareNotEqual_128(n0, n1, 0, INT64_MIN)));

	// extract signs
	uint64_t sn = uint64_t(n1 >> 63);
	uint64_t sd = uint64_t(d >> 63);

	// transform inputs
	uint64_t un0, un1;
	Subtract_128(n0 ^ sn, uint64_t(n1) ^ sn, sn, sn, un0, un1);
	uint64_t ud = (uint64_t(d) ^ sd) - sd;

	// add offset
	uint64_t sq = sn ^ sd;
	uint64_t offset = (ud + sq) >> 1;
	Add_128(un0, un1, offset, 0, un0, un1);

	// do unsigned division
	uint64_t uq0, uq1, ur;
	DivideFloor_128x64_128(un0, un1, ud, uq0, uq1, ur);

	// transform outputs
	Subtract_128(uq0 ^ sq, int64_t(uq1 ^ sq), sq, int64_t(sq), q0, q1);
	r = int64_t(((ur - offset) ^ sn) - sn);

}

inline void DivideCeil_128x64_128(uint64_t n0, int64_t n1, int64_t d, uint64_t &q0, int64_t &q1, int64_t &r) {
	// division by zero or divisions where the result overflows are forbidden
	// q = ceil(n / d), r = n - d * q
	assert(d != 0 && (d != -1 || CompareNotEqual_128(n0, n1, 0, INT64_MIN)));

	// extract signs
	uint64_t sn = uint64_t(n1 >> 63);
	uint64_t sd = uint64_t(d >> 63);

	// transform inputs
	uint64_t un0, un1;
	Subtract_128(n0 ^ sn, uint64_t(n1) ^ sn, sn, sn, un0, un1);
	uint64_t ud = (uint64_t(d) ^ sd) - sd;

	// add offset
	uint64_t sq = sn ^ sd;
	uint64_t offset = (ud - 1) & (~sq);
	Add_128(un0, un1, offset, 0, un0, un1);

	// do unsigned division
	uint64_t uq0, uq1, ur;
	DivideFloor_128x64_128(un0, un1, ud, uq0, uq1, ur);

	// transform outputs
	Subtract_128(uq0 ^ sq, int64_t(uq1 ^ sq), sq, int64_t(sq), q0, q1);
	r = int64_t(((ur - offset) ^ sn) - sn);

}

}
