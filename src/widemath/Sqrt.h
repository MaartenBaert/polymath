#pragma once

#include "Common.h"

#include "Compare.h"
#include "Shift.h"
#include "Subtract.h"

namespace WideMath {

inline uint32_t SqrtFloor_32(uint32_t x) {
	uint32_t r = 0;
	uint32_t b = uint32_t(1) << 30;
	while(b != 0) {
		uint32_t rb = r + b;
		r >>= 1;
		if(x >= rb) {
			x -= rb;
			r += b;
		}
		b >>= 2;
	}
	return r;
}

inline uint64_t SqrtFloor_64(uint64_t x) {
	uint64_t r = 0;
	uint64_t b = uint64_t(1) << 62;
	while(b != 0) {
		uint64_t rb = r + b;
		r >>= 1;
		if(x >= rb) {
			x -= rb;
			r += b;
		}
		b >>= 2;
	}
	return r;
}

inline uint64_t SqrtFloor_128_64(uint64_t x0, uint64_t x1) {
	uint64_t r0 = 0, r1 = 0;
	uint64_t b1 = uint64_t(1) << 62;
	while(b1 != 0) {
		uint64_t rb1 = r1 + b1;
		r1 >>= 1;
		if(x1 >= rb1) {
			x1 -= rb1;
			r1 += b1;
		}
		b1 >>= 2;
	}
	uint64_t b0 = uint64_t(1) << 62;
	while(b0 != 0) {
		uint64_t rb0 = r0 + b0, rb1 = r1;
		r0 = ShiftRight_128_64(r0, r1, 1);
		r1 >>= 1;
		if(CompareGreaterEqual_128(x0, x1, rb0, rb1)) {
			Subtract_128(x0, x1, rb0, rb1, x0, x1);
			r0 += b0;
		}
		b0 >>= 2;
	}
	return r0;
}

inline uint32_t SqrtRound_32(uint32_t x) {
	// input range = 0 .. 2**32 - 2**16
	assert(x <= (uint32_t(UINT16_MAX) << 16));
	uint32_t r = 0;
	uint32_t b = uint32_t(1) << 30;
	while(b != 0) {
		uint32_t rb = r + b;
		r >>= 1;
		if(x >= rb) {
			x -= rb;
			r += b;
		}
		b >>= 2;
	}
	if(x > r) {
		++r;
	}
	return r;
}

inline uint64_t SqrtRound_64(uint64_t x) {
	// input range = 0 .. 2**64 - 2**32
	assert(x <= (uint64_t(UINT32_MAX) << 32));
	uint64_t r = 0;
	uint64_t b = uint64_t(1) << 62;
	while(b != 0) {
		uint64_t rb = r + b;
		r >>= 1;
		if(x >= rb) {
			x -= rb;
			r += b;
		}
		b >>= 2;
	}
	if(x > r) {
		++r;
	}
	return r;
}

inline uint64_t SqrtRound_128_64(uint64_t x0, uint64_t x1) {
	// input range = 0 .. 2**128 - 2**64
	// output range = 0 .. 2**64 - 1
	assert(x1 != UINT64_MAX || x0 == 0);
	uint64_t r0 = 0, r1 = 0;
	uint64_t b1 = uint64_t(1) << 62;
	while(b1 != 0) {
		uint64_t rb1 = r1 + b1;
		r1 >>= 1;
		if(x1 >= rb1) {
			x1 -= rb1;
			r1 += b1;
		}
		b1 >>= 2;
	}
	uint64_t b0 = uint64_t(1) << 62;
	while(b0 != 0) {
		uint64_t rb0 = r0 + b0, rb1 = r1;
		r0 = ShiftRight_128_64(r0, r1, 1);
		r1 >>= 1;
		if(CompareGreaterEqual_128(x0, x1, rb0, rb1)) {
			Subtract_128(x0, x1, rb0, rb1, x0, x1);
			r0 += b0;
		}
		b0 >>= 2;
	}
	r0 += CompareGreater_128(x0, x1, r0, r1);
	return r0;
}

}
