#pragma once

#include "Common.h"

#include "widemath/WideMath.h"

namespace PolyMath {

template<int bits, typename I1, typename I2, typename I4>
struct NumericalEngine_Int {

	typedef I1 SingleType;
	typedef I2 DoubleType;

	// Convert single to double precision.
	static I2 SingleToDouble(I1 x) {
		return I2(x) << bits;
	}

	// Convert double to single precision.
	static I1 DoubleToSingle(I2 x) {
		return I1((x + (I2(1) << (bits - 1)) - 1) >> bits);
	}

	// Returns whether (a, b, c) are in clockwise order (left-handed) or counter-clockwise order (right-handed).
	static bool OrientationTest(I1 a_x, I1 a_y, I1 b_x, I1 b_y, I1 c_x, I1 c_y, bool strict) {
		I2 lhs = I2(b_x - a_x) * I2(c_y - a_y);
		I2 rhs = I2(b_y - a_y) * I2(c_x - a_x);
		return (strict)? (lhs > rhs) : (lhs >= rhs);
	}

	// Returns whether two edges intersect and calculates the intersection point if they do.
	static bool IntersectionTest(I1 a1_x, I1 a1_y, I1 a2_x, I1 a2_y, I1 b1_x, I1 b1_y, I1 b2_x, I1 b2_y, I2 &res_x, I2 &res_y) {
		if(a2_x < b2_x) {
			I2 num = I2(b2_x - b1_x) * I2(a2_y - b2_y) - I2(b2_y - b1_y) * I2(a2_x - b2_x);
			if(num > I2(0)) {
				I2 den = I2(b2_x - b1_x) * I2(a2_y - a1_y) - I2(b2_y - b1_y) * I2(a2_x - a1_x);
				if(den > I2(0)) {
					I4 nx = I4(SingleToDouble(a2_x - a1_x)) * I4(num);
					I4 ny = I4(SingleToDouble(a2_y - a1_y)) * I4(num) - I4((den - 1) & (I2(a2_y - a1_y) >> (2 * bits - 1)));
					res_x = SingleToDouble(a2_x) - I2(nx / I4(den));
					res_y = SingleToDouble(a2_y) - I2(ny / I4(den));
				} else {
					res_x = SingleToDouble(a2_x);
					res_y = SingleToDouble(a2_y);
				}
				return true;
			}
		} else {
			I2 num = I2(a2_y - a1_y) * I2(b2_x - a2_x) - I2(a2_x - a1_x) * I2(b2_y - a2_y);
			if(num > I2(0)) {
				I2 den = I2(a2_y - a1_y) * I2(b2_x - b1_x) - I2(a2_x - a1_x) * I2(b2_y - b1_y);
				if(den > I2(0)) {
					I4 nx = I4(SingleToDouble(b2_x - b1_x)) * I4(num);
					I4 ny = I4(SingleToDouble(b2_y - b1_y)) * I4(num) - I4((den - 1) & (I2(b2_y - b1_y) >> (2 * bits - 1)));
					res_x = SingleToDouble(b2_x) - I2(nx / I4(den));
					res_y = SingleToDouble(b2_y) - I2(ny / I4(den));
				} else {
					res_x = SingleToDouble(b2_x);
					res_y = SingleToDouble(b2_y);
				}
				return true;
			}
		}
		return false;
	}

};

struct NumericalEngine_Int32 {

	typedef int32_t SingleType;
	typedef int64_t DoubleType;

	// Convert single to double precision.
	static int64_t SingleToDouble(int32_t x) {
		return int64_t(x) << 32;
	}

	// Convert double to single precision.
	static int32_t DoubleToSingle(int64_t x) {
		return int32_t((x + (int64_t(1) << (32 - 1)) - 1) >> 32);
	}

	// Returns whether (a, b, c) are in clockwise order (left-handed) or counter-clockwise order (right-handed).
	static bool OrientationTest(int32_t a_x, int32_t a_y, int32_t b_x, int32_t b_y, int32_t c_x, int32_t c_y, bool strict) {
		int64_t lhs = int64_t(b_x - a_x) * int64_t(c_y - a_y);
		int64_t rhs = int64_t(b_y - a_y) * int64_t(c_x - a_x);
		return (strict)? (lhs > rhs) : (lhs >= rhs);
	}

	// Returns whether two edges intersect and calculates the intersection point if they do.
	static bool IntersectionTest(int32_t a1_x, int32_t a1_y, int32_t a2_x, int32_t a2_y, int32_t b1_x, int32_t b1_y, int32_t b2_x, int32_t b2_y, int64_t &res_x, int64_t &res_y) {
		if(a2_x < b2_x) {
			int64_t num = int64_t(b2_x - b1_x) * int64_t(a2_y - b2_y) - int64_t(b2_y - b1_y) * int64_t(a2_x - b2_x);
			if(num > int64_t(0)) {
				int64_t den = int64_t(b2_x - b1_x) * int64_t(a2_y - a1_y) - int64_t(b2_y - b1_y) * int64_t(a2_x - a1_x);
				if(den > int64_t(0)) {
					uint64_t nx0, ny0;
					int64_t nx1, ny1;
					WideMath::Multiply_64x64_128(SingleToDouble(a2_x - a1_x), num, nx0, nx1);
					WideMath::Multiply_64x64_128(SingleToDouble(a2_y - a1_y), num, ny0, ny1);
					WideMath::Subtract_128(ny0, ny1, uint64_t((den - 1) & (int64_t(a2_y - a1_y) >> 63)), 0, ny0, ny1);
					int64_t qx, qy, rx, ry;
					WideMath::DivideTrunc_128x64_64(nx0, nx1, den, qx, rx);
					WideMath::DivideTrunc_128x64_64(ny0, ny1, den, qy, ry);
					res_x = SingleToDouble(a2_x) - qx;
					res_y = SingleToDouble(a2_y) - qy;
				} else {
					res_x = SingleToDouble(a2_x);
					res_y = SingleToDouble(a2_y);
				}
				return true;
			}
		} else {
			int64_t num = int64_t(a2_y - a1_y) * int64_t(b2_x - a2_x) - int64_t(a2_x - a1_x) * int64_t(b2_y - a2_y);
			if(num > int64_t(0)) {
				int64_t den = int64_t(a2_y - a1_y) * int64_t(b2_x - b1_x) - int64_t(a2_x - a1_x) * int64_t(b2_y - b1_y);
				if(den > int64_t(0)) {
					uint64_t nx0, ny0;
					int64_t nx1, ny1;
					WideMath::Multiply_64x64_128(SingleToDouble(b2_x - b1_x), num, nx0, nx1);
					WideMath::Multiply_64x64_128(SingleToDouble(b2_y - b1_y), num, ny0, ny1);
					WideMath::Subtract_128(ny0, ny1, uint64_t((den - 1) & (int64_t(b2_y - b1_y) >> 63)), 0, ny0, ny1);
					int64_t qx, qy, rx, ry;
					WideMath::DivideTrunc_128x64_64(nx0, nx1, den, qx, rx);
					WideMath::DivideTrunc_128x64_64(ny0, ny1, den, qy, ry);
					res_x = SingleToDouble(b2_x) - qx;
					res_y = SingleToDouble(b2_y) - qy;
				} else {
					res_x = SingleToDouble(b2_x);
					res_y = SingleToDouble(b2_y);
				}
				return true;
			}
		}
		return false;
	}

};

struct NumericalEngine_Int64 {

	struct Int128 {
		uint64_t v0;
		int64_t v1;
		friend bool operator>(Int128 a, Int128 b) { return WideMath::CompareGreater_128(a.v0, a.v1, b.v0, b.v1); }
		friend bool operator<(Int128 a, Int128 b) { return WideMath::CompareLess_128(a.v0, a.v1, b.v0, b.v1); }
		friend bool operator>=(Int128 a, Int128 b) { return WideMath::CompareGreaterEqual_128(a.v0, a.v1, b.v0, b.v1); }
		friend bool operator<=(Int128 a, Int128 b) { return WideMath::CompareLessEqual_128(a.v0, a.v1, b.v0, b.v1); }
	};

	typedef int64_t SingleType;
	typedef Int128 DoubleType;

	// Convert single to double precision.
	static Int128 SingleToDouble(int64_t x) {
		return Int128{0, x};
	}

	// Convert double to single precision.
	static int64_t DoubleToSingle(Int128 x) {
		uint64_t v0;
		int64_t v1;
		WideMath::Add_128(x.v0, x.v1, uint64_t(1) << 63, 0, v0, v1);
		return v1;
	}

	// Returns whether (a, b, c) are in clockwise order (left-handed) or counter-clockwise order (right-handed).
	static bool OrientationTest(int64_t a_x, int64_t a_y, int64_t b_x, int64_t b_y, int64_t c_x, int64_t c_y, bool strict) {
		uint64_t lhs0, rhs0;
		int64_t lhs1, rhs1;
		WideMath::Multiply_64x64_128(b_x - a_x, c_y - a_y, lhs0, lhs1);
		WideMath::Multiply_64x64_128(b_y - a_y, c_x - a_x, rhs0, rhs1);
		return (strict)? WideMath::CompareGreater_128(lhs0, lhs1, rhs0, rhs1) : WideMath::CompareGreaterEqual_128(lhs0, lhs1, rhs0, rhs1);
	}

	// Returns whether two edges intersect and calculates the intersection point if they do.
	static bool IntersectionTest(int64_t a1_x, int64_t a1_y, int64_t a2_x, int64_t a2_y, int64_t b1_x, int64_t b1_y, int64_t b2_x, int64_t b2_y, Int128 &res_x, Int128 &res_y) {
		if(a2_x < b2_x) {
			uint64_t num0, num00, num10;
			int64_t num1, num01, num11;
			WideMath::Multiply_64x64_128(b2_x - b1_x, a2_y - b2_y, num00, num01);
			WideMath::Multiply_64x64_128(b2_y - b1_y, a2_x - b2_x, num10, num11);
			WideMath::Subtract_128(num00, num01, num10, num11, num0, num1);
			if(WideMath::CompareGreater_128(num0, num1, 0, 0)) {
				uint64_t den0, den00, den10;
				int64_t den1, den01, den11;
				WideMath::Multiply_64x64_128(b2_x - b1_x, a2_y - a1_y, den00, den01);
				WideMath::Multiply_64x64_128(b2_y - b1_y, a2_x - a1_x, den10, den11);
				WideMath::Subtract_128(den00, den01, den10, den11, den0, den1);
				if(WideMath::CompareGreater_128(den0, den1, 0, 0)) {
					int64_t dx = a2_x - a1_x, dy = a2_y - a1_y;
					uint64_t sy = uint64_t(dy >> 63);
					uint64_t udx = uint64_t(dx), udy = (uint64_t(dy) ^ sy) - sy;
					uint64_t nx0, ny0, nx1, ny1, nx2, ny2, nx3, ny3;
					WideMath::Multiply_128x128_256(0, udx, num0, uint64_t(num1), nx0, nx1, nx2, nx3);
					WideMath::Multiply_128x128_256(0, udy, num0, uint64_t(num1), ny0, ny1, ny2, ny3);
					uint64_t off0, off1;
					WideMath::Subtract_128(den0, uint64_t(den1), 1, 0, off0, off1);
					WideMath::Add_256(ny0, ny1, ny2, ny3, off0 & sy, off1 & sy, 0, 0, ny0, ny1, ny2, ny3);
					uint64_t uqx0, uqx1, uqy0, uqy1, urx0, urx1, ury0, ury1;
					WideMath::DivideFloor_256x128_128(nx0, nx1, nx2, nx3, den0, uint64_t(den1), uqx0, uqx1, urx0, urx1);
					WideMath::DivideFloor_256x128_128(ny0, ny1, ny2, ny3, den0, uint64_t(den1), uqy0, uqy1, ury0, ury1);
					uint64_t qx0 = uqx0, qy0;
					int64_t qx1 = int64_t(uqx1), qy1;
					WideMath::Subtract_128(uqy0 ^ sy, int64_t(uqy1 ^ sy), sy, int64_t(sy), qy0, qy1);
					WideMath::Subtract_128(0, a2_x, qx0, qx1, res_x.v0, res_x.v1);
					WideMath::Subtract_128(0, a2_y, qy0, qy1, res_y.v0, res_y.v1);
				} else {
					res_x = Int128{0, a2_x};
					res_y = Int128{0, a2_y};
				}
				return true;
			}
		} else {
			uint64_t num0, num00, num10;
			int64_t num1, num01, num11;
			WideMath::Multiply_64x64_128(a2_y - a1_y, b2_x - a2_x, num00, num01);
			WideMath::Multiply_64x64_128(a2_x - a1_x, b2_y - a2_y, num10, num11);
			WideMath::Subtract_128(num00, num01, num10, num11, num0, num1);
			if(WideMath::CompareGreater_128(num0, num1, 0, 0)) {
				uint64_t den0, den00, den10;
				int64_t den1, den01, den11;
				WideMath::Multiply_64x64_128(a2_y - a1_y, b2_x - b1_x, den00, den01);
				WideMath::Multiply_64x64_128(a2_x - a1_x, b2_y - b1_y, den10, den11);
				WideMath::Subtract_128(den00, den01, den10, den11, den0, den1);
				if(WideMath::CompareGreater_128(den0, den1, 0, 0)) {
					int64_t dx = b2_x - b1_x, dy = b2_y - b1_y;
					uint64_t sy = uint64_t(dy >> 63);
					uint64_t udx = uint64_t(dx), udy = (uint64_t(dy) ^ sy) - sy;
					uint64_t nx0, ny0, nx1, ny1, nx2, ny2, nx3, ny3;
					WideMath::Multiply_128x128_256(0, udx, num0, uint64_t(num1), nx0, nx1, nx2, nx3);
					WideMath::Multiply_128x128_256(0, udy, num0, uint64_t(num1), ny0, ny1, ny2, ny3);
					uint64_t off0, off1;
					WideMath::Subtract_128(den0, uint64_t(den1), 1, 0, off0, off1);
					WideMath::Add_256(ny0, ny1, ny2, ny3, off0 & sy, off1 & sy, 0, 0, ny0, ny1, ny2, ny3);
					uint64_t uqx0, uqx1, uqy0, uqy1, urx0, urx1, ury0, ury1;
					WideMath::DivideFloor_256x128_128(nx0, nx1, nx2, nx3, den0, den1, uqx0, uqx1, urx0, urx1);
					WideMath::DivideFloor_256x128_128(ny0, ny1, ny2, ny3, den0, den1, uqy0, uqy1, ury0, ury1);
					uint64_t qx0 = uqx0, qy0;
					int64_t qx1 = int64_t(uqx1), qy1;
					WideMath::Subtract_128(uqy0 ^ sy, int64_t(uqy1 ^ sy), sy, int64_t(sy), qy0, qy1);
					WideMath::Subtract_128(0, b2_x, qx0, qx1, res_x.v0, res_x.v1);
					WideMath::Subtract_128(0, b2_y, qy0, qy1, res_y.v0, res_y.v1);
				} else {
					res_x = Int128{0, b2_x};
					res_y = Int128{0, b2_y};
				}
				return true;
			}
		}
		return false;
	}

};

template<typename F1, typename F2>
struct NumericalEngine_Float {

	typedef F1 SingleType;
	typedef F2 DoubleType;

	// Convert single to double precision.
	static F2 SingleToDouble(F1 x) {
		return F2(x);
	}

	// Convert double to single precision.
	static F1 DoubleToSingle(F2 x) {
		return F1(x);
	}

	// Returns whether (a, b, c) are in clockwise order (left-handed) or counter-clockwise order (right-handed).
	static bool OrientationTest(F1 a_x, F1 a_y, F1 b_x, F1 b_y, F1 c_x, F1 c_y, bool strict) {
		F2 lhs = (F2(b_x) - F2(a_x)) * (F2(c_y) - F2(a_y));
		F2 rhs = (F2(b_y) - F2(a_y)) * (F2(c_x) - F2(a_x));
		return (strict)? (lhs > rhs) : (lhs >= rhs);
	}

	// Returns whether two edges intersect and calculates the intersection point if they do.
	static bool IntersectionTest(F1 a1_x, F1 a1_y, F1 a2_x, F1 a2_y, F1 b1_x, F1 b1_y, F1 b2_x, F1 b2_y, F2 &res_x, F2 &res_y) {
		if(a2_x < b2_x) {
			F2 num = (F2(b2_x) - F2(b1_x)) * (F2(a2_y) - F2(b2_y)) - (F2(b2_y) - F2(b1_y)) * (F2(a2_x) - F2(b2_x));
			if(num > F2(0)) {
				F2 den = (F2(b2_x) - F2(b1_x)) * (F2(a2_y) - F2(a1_y)) - (F2(b2_y) - F2(b1_y)) * (F2(a2_x) - F2(a1_x));
				if(den > F2(0)) {
					F2 t = num / den;
					res_x = F2(a2_x) + (F2(a1_x) - F2(a2_x)) * t;
					res_y = F2(a2_y) + (F2(a1_y) - F2(a2_y)) * t;
				} else {
					res_x = F2(a2_x);
					res_y = F2(a2_y);
				}
				return true;
			}
		} else {
			F2 num = (F2(a2_y) - F2(a1_y)) * (F2(b2_x) - F2(a2_x)) - (F2(a2_x) - F2(a1_x)) * (F2(b2_y) - F2(a2_y));
			if(num > F2(0)) {
				F2 den = (F2(a2_y) - F2(a1_y)) * (F2(b2_x) - F2(b1_x)) - (F2(a2_x) - F2(a1_x)) * (F2(b2_y) - F2(b1_y));
				if(den > F2(0)) {
					F2 t = num / den;
					res_x = F2(b2_x) + (F2(b1_x) - F2(b2_x)) * t;
					res_y = F2(b2_y) + (F2(b1_y) - F2(b2_y)) * t;
				} else {
					res_x = F2(b2_x);
					res_y = F2(b2_y);
				}
				return true;
			}
		}
		return false;
	}

};

template<typename T>
struct NumericalEngine;

template<> struct NumericalEngine<int8_t > : NumericalEngine_Int<8, int8_t, int16_t, int32_t> {};
template<> struct NumericalEngine<int16_t> : NumericalEngine_Int<16, int16_t, int32_t, int64_t> {};
template<> struct NumericalEngine<int32_t> : NumericalEngine_Int32 {};
template<> struct NumericalEngine<int64_t> : NumericalEngine_Int64 {};

template<> struct NumericalEngine<float  > : NumericalEngine_Float<float, double> {};
#ifdef __SIZEOF_FLOAT128__
template<> struct NumericalEngine<double > : NumericalEngine_Float<double, __float128> {};
#endif

}
