#pragma once

#include "Common.h"

#include <ostream>

namespace PolyMath {

template<typename T>
struct Vertex {

	typedef T ValueType;

	T x, y;

	inline Vertex() {}
	inline Vertex(T x, T y) : x(x), y(y) {}

	friend std::ostream& operator<<(std::ostream &stream, const Vertex &v) {
		return stream << '(' << v.x << ',' << v.y << ')';
	}

};

}

