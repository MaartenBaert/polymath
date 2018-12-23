#pragma once

#include "Common.h"

#include "Vertex.h"

namespace PolyMath {

template<class Vertex, typename WindingNumber = int32_t>
class Polygon {

public:
	typedef Vertex VertexType;
	typedef WindingNumber WindingNumberType;

private:
	struct Loop {
		size_t m_end;
		WindingNumber m_winding_weight;
		Loop(size_t end, WindingNumber winding_weight) : m_end(end), m_winding_weight(winding_weight) {}
	};

private:
	std::vector<Vertex> m_vertices;
	std::vector<Loop> m_loops;

public:
	Polygon() = default;
	Polygon(const Polygon &other) = default;
	Polygon(Polygon &&other) = default;
	Polygon(std::initializer_list<std::initializer_list<Vertex>> data) {
		for(auto loop : data) {
			for(Vertex v : loop) {
				AddVertex(v);
			}
			AddLoopEnd(1);
		}
	}

	Polygon& operator=(const Polygon &other) = default;
	Polygon& operator=(Polygon &&other) = default;
	Polygon& operator=(std::initializer_list<std::initializer_list<Vertex>> data) {
		Clear();
		for(auto loop : data) {
			for(Vertex v : loop) {
				AddVertex(v);
			}
			AddLoopEnd(1);
		}
		return *this;
	}

	Polygon& operator+=(const Polygon &other) {
		for(size_t i = 0; i < other.GetLoopCount(); i++) {
			const Vertex *vertices = other.GetLoopVertices(i);
			size_t n = other.GetLoopVertexCount(i);
			for(size_t j = 0; j < n; ++j) {
				AddVertex(vertices[j]);
			}
			AddLoopEnd(other.GetLoopWindingWeight(i));
		}
		return *this;
	}

	Polygon& operator-=(const Polygon &other) {
		for(size_t i = 0; i < other.GetLoopCount(); i++) {
			const Vertex *vertices = other.GetLoopVertices(i);
			size_t n = other.GetLoopVertexCount(i);
			for(size_t j = 0; j < n; ++j) {
				AddVertex(vertices[j]);
			}
			AddLoopEnd(-other.GetLoopWindingWeight(i));
		}
		return *this;
	}

	friend Polygon& operator+(const Polygon &a, const Polygon &b) { return Polygon(a) += b; }
	friend Polygon& operator+(Polygon &&a, const Polygon &b) { return  a += b; }
	friend Polygon& operator-(const Polygon &a, const Polygon &b) { return Polygon(a) -= b; }
	friend Polygon& operator-(Polygon &&a, const Polygon &b) { return  a -= b; }

public:
	inline void Clear() {
		m_vertices.clear();
		m_loops.clear();
	}
	inline void ReserveVertices(size_t count) {
		m_vertices.reserve(count);
	}
	inline void ReserveLoops(size_t count) {
		m_loops.reserve(count);
	}
	inline void AddVertex(Vertex v) {
		m_vertices.push_back(v);
	}
	inline void AddLoopEnd(WindingNumber winding_weight) {
		m_loops.emplace_back(m_vertices.size(), winding_weight);
	}
	inline void AddLoop(const Vertex *vertices, size_t vertex_count, WindingNumber winding_weight) {
		m_vertices.insert(m_vertices.end(), vertices, vertices + vertex_count);
		AddLoopEnd(winding_weight);
	}

public:
	inline size_t GetVertexCount() const {
		return m_vertices.size();
	}
	inline size_t GetLoopCount() const {
		return m_loops.size();
	}
	inline Vertex* GetVertices() {
		return m_vertices.data();
	}
	inline const Vertex* GetVertices() const {
		return m_vertices.data();
	}
	inline Vertex& GetVertex(size_t i) {
		assert(i < m_vertices.size());
		return m_vertices[i];
	}
	inline const Vertex& GetVertex(size_t i) const {
		assert(i < m_vertices.size());
		return m_vertices[i];
	}
	inline size_t GetLoopEnd(size_t i) const {
		assert(i < m_loops.size());
		return m_loops[i].m_end;
	}
	inline WindingNumber GetLoopWindingWeight(size_t i) const {
		assert(i < m_loops.size());
		return m_loops[i].m_winding_weight;
	}
	inline Vertex* GetLoopVertices(size_t i) {
		assert(i < m_loops.size());
		return (i == 0)? m_vertices.data() : m_vertices.data() + m_loops[i - 1].m_end;
	}
	inline const Vertex* GetLoopVertices(size_t i) const {
		assert(i < m_loops.size());
		return (i == 0)? m_vertices.data() : m_vertices.data() + m_loops[i - 1].m_end;
	}
	inline size_t GetLoopVertexCount(size_t i) const {
		assert(i < m_loops.size());
		return (i == 0)? m_loops[i].m_end : m_loops[i].m_end - m_loops[i - 1].m_end;
	}

};

}
