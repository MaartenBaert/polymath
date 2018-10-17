#pragma once

#include "Common.h"

namespace PolyMath {

class WindingEngine_Union {
public:
	static bool WindingRule(int64_t x) {
		return (x > 0);
	}
};

class WindingEngine_Intersection {
public:
	static bool WindingRule(int64_t x) {
		return (x > 1);
	}
};

class WindingEngine_Xor {
public:
	static bool WindingRule(int64_t x) {
		return bool(x & 1);
	}
};

}
