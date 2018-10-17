#pragma once

#include <ostream>

#include "Common.h"

namespace PolyMath {

template<typename F>
class Vertex {

public:
	typedef F value_type;

private:
	F m_x, m_y;

public:
	inline Vertex() {}
	inline Vertex(F x, F y) : m_x(x), m_y(y) {}

	inline F x() const { return m_x; }
	inline F y() const { return m_y; }

	inline void x(F x) { m_x = x; }
	inline void y(F y) { m_y = y; }

	friend std::ostream& operator<<(std::ostream &stream, const Vertex &v) {
		return stream << '(' << v.x() << ',' << v.y() << ')';
	}

};

}
