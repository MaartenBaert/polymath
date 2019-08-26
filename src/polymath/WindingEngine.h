#pragma once

#include "Common.h"

namespace PolyMath {

template<typename W = int32_t>
struct WindingEngine_NonZero {
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool WindingRule(WindingNumberType x) {
		return (x != 0);
	}
};

template<typename W = int32_t>
struct WindingEngine_EvenOdd {
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool WindingRule(WindingNumberType x) {
		return bool(x & 1);
	}
};

template<typename W = int32_t>
struct WindingEngine_Positive {
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool WindingRule(WindingNumberType x) {
		return (x > 0);
	}
};

template<typename W = int32_t>
struct WindingEngine_Negative {
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool WindingRule(WindingNumberType x) {
		return (x < 0);
	}
};

}
