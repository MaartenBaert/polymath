#pragma once

#include "Common.h"

namespace PolyMath {

enum WindingRule {
	WINDINGRULE_NONZERO,
	WINDINGRULE_EVENODD,
	WINDINGRULE_POSITIVE,
	WINDINGRULE_NEGATIVE,
};

template<typename W = default_winding_t>
class WindingPolicy_Dynamic {
private:
	WindingRule m_winding_rule;
public:
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	WindingPolicy_Dynamic(WindingRule winding_rule)
		: m_winding_rule(winding_rule) {}
	bool Evaluate(WindingNumberType x) {
		switch(m_winding_rule) {
			case WINDINGRULE_NONZERO: return (x != 0);
			case WINDINGRULE_EVENODD: return bool(x & 1);
			case WINDINGRULE_POSITIVE: return (x > 0);
			case WINDINGRULE_NEGATIVE: return (x < 0);
		}
	}
};

template<typename W = default_winding_t>
class WindingPolicy_NonZero {
public:
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool Evaluate(WindingNumberType x) {
		return (x != 0);
	}
};

template<typename W = default_winding_t>
class WindingPolicy_EvenOdd {
public:
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool Evaluate(WindingNumberType x) {
		return bool(x & 1);
	}
};

template<typename W = default_winding_t>
class WindingPolicy_Positive {
public:
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool Evaluate(WindingNumberType x) {
		return (x > 0);
	}
};

template<typename W = default_winding_t>
class WindingPolicy_Negative {
public:
	typedef W WindingNumberType;
	typedef W WindingWeightType;
	static bool Evaluate(WindingNumberType x) {
		return (x < 0);
	}
};

}
