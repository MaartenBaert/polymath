#pragma once

#include "Common.h"

namespace PolyMath {

template<typename T> struct DoublePrecision;

template<> struct DoublePrecision<int8_t> { typedef int16_t value; };
template<> struct DoublePrecision<int16_t> { typedef int32_t value; };
template<> struct DoublePrecision<int32_t> { typedef int64_t value; };
#ifdef __SIZEOF_INT128__
template<> struct DoublePrecision<int64_t> { typedef __int128 value; };
#endif

template<> struct DoublePrecision<float> { typedef double value; };
#ifdef __SIZEOF_FLOAT128__
template<> struct DoublePrecision<double> { typedef __float128 value; };
#endif

template<typename T>
class NumericalEngine {

public:
	typedef T single_type;
	typedef typename DoublePrecision<T>::value double_type;

public:

	// Returns whether (a, b, c) are in clockwise order.
	static bool OrientationTest(single_type a_x, single_type a_y, single_type b_x, single_type b_y, single_type c_x, single_type c_y) {
		double_type aa_x = double_type(a_x), aa_y = double_type(a_y);
		double_type bb_x = double_type(b_x), bb_y = double_type(b_y);
		double_type cc_x = double_type(c_x), cc_y = double_type(c_y);
		return ((bb_x - aa_x) * (cc_y - aa_y) > (bb_y - aa_y) * (cc_x - aa_x));
	}

	// Returns whether two edges intersect and calculates the intersection point if they do.
	static bool IntersectionTest(
			single_type a1_x, single_type a1_y, single_type a2_x, single_type a2_y,
			single_type b1_x, single_type b1_y, single_type b2_x, single_type b2_y,
			double_type &res_x, double_type &res_y) {
		double_type aa1_x = double_type(a1_x), aa1_y = double_type(a1_y), aa2_x = double_type(a2_x), aa2_y = double_type(a2_y);
		double_type bb1_x = double_type(b1_x), bb1_y = double_type(b1_y), bb2_x = double_type(b2_x), bb2_y = double_type(b2_y);
		if(a2_y < b2_y) {
			double_type num = (bb2_x - bb1_x) * (aa2_y - bb1_y) - (bb2_y - bb1_y) * (aa2_x - bb1_x);
			if(num < double_type(0)) {
				double_type den = (bb2_x - bb1_x) * (aa2_y - aa1_y) - (bb2_y - bb1_y) * (aa2_x - aa1_x);
				assert(den < double_type(0));
				assert(num <= double_type(0));
				assert(num >= den);
				double_type t = num / den;
				res_x = aa2_x + (aa1_x - aa2_x) * t;
				res_y = aa2_y + (aa1_y - aa2_y) * t;
				return true;
			}
		} else {
			double_type num = (aa2_x - aa1_x) * (bb2_y - aa1_y) - (aa2_y - aa1_y) * (bb2_x - aa1_x);
			if(num > double_type(0)) {
				double_type den = (aa2_x - aa1_x) * (bb2_y - bb1_y) - (aa2_y - aa1_y) * (bb2_x - bb1_x);
				assert(den > double_type(0));
				assert(num >= double_type(0));
				assert(num <= den);
				double_type t = num / den;
				res_x = bb2_x + (bb1_x - bb2_x) * t;
				res_y = bb2_y + (bb1_y - bb2_y) * t;
				return true;
			}
		}
		return false;
	}

};

}
