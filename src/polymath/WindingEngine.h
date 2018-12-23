#pragma once

#include "Common.h"

namespace PolyMath {

template<typename WindingNumber = int32_t>
struct WindingEngine_NonZero {

	typedef WindingNumber WindingNumberType;

	static bool WindingRule(WindingNumber x) {
		return (x != 0);
	}

};

template<typename WindingNumber = int32_t>
struct WindingEngine_EvenOdd {

	typedef WindingNumber WindingNumberType;

	static bool WindingRule(WindingNumber x) {
		return bool(x & 1);
	}

};

template<typename WindingNumber = int32_t>
struct WindingEngine_Positive {

	typedef WindingNumber WindingNumberType;

	static bool WindingRule(WindingNumber x) {
		return (x > 0);
	}

};

template<typename WindingNumber = int32_t>
struct WindingEngine_Negative {

	typedef WindingNumber WindingNumberType;

	static bool WindingRule(WindingNumber x) {
		return (x < 0);
	}

};

}
