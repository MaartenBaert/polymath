#pragma once

#include "Common.h"

#include "Vertex.h"

#include <algorithm>

namespace PolyMath {

template<class Vertex, typename WindingNumber = int32_t>
struct Polygon {

	typedef Vertex VertexType;
	typedef WindingNumber WindingNumberType;

	struct Loop {
		size_t end;
		WindingNumber weight;
		Loop() {}
		Loop(size_t end, WindingNumber weight) : end(end), weight(weight) {}
	};

	std::vector<Vertex> vertices;
	std::vector<Loop> loops;

	Polygon() = default;
	Polygon(const Polygon &other) = default;
	Polygon(Polygon &&other) = default;

	Polygon(const std::vector<Vertex> &vertices, const std::vector<Loop> &loops)
		: vertices(vertices), loops(loops) {}
	Polygon(Vertex *vertices, size_t vertex_count, Loop *loops, size_t loop_count)
		: vertices(vertices, vertices + vertex_count), loops(loops, loops + loop_count) {}

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
		size_t v1 = vertices.size(), v2 = other.vertices.size();
		size_t l1 = loops.size(), l2 = other.loops.size();
		vertices.resize(v1 + v2);
		loops.resize(l1 + l2);
		std::copy_n(other.vertices.data(), v2, vertices.data() + v1);
		for(size_t i = 0; i < l2; ++i) {
			loops[l1 + i].end = other.loops[i].end + v1;
			loops[l1 + i].weight = other.loops[i].weight;
		}
		return *this;
	}

	Polygon& operator-=(const Polygon &other) {
		size_t v1 = vertices.size(), v2 = other.vertices.size();
		size_t l1 = loops.size(), l2 = other.loops.size();
		vertices.resize(v1 + v2);
		loops.resize(l1 + l2);
		std::copy_n(other.vertices.data(), v2, vertices.data() + v1);
		for(size_t i = 0; i < l2; ++i) {
			loops[l1 + i].end = other.loops[i].end + v1;
			loops[l1 + i].weight = -other.loops[i].weight;
		}
		return *this;
	}

	friend Polygon operator+(const Polygon &a, const Polygon &b) { return Polygon(a) += b; }
	friend Polygon operator+(Polygon &&a, const Polygon &b) { return std::move(a += b); }
	friend Polygon operator-(const Polygon &a, const Polygon &b) { return Polygon(a) -= b; }
	friend Polygon operator-(Polygon &&a, const Polygon &b) { return std::move(a -= b); }

	bool IsValid() const {
		size_t last = 0;
		for(size_t i = 0; i < loops.size(); ++i) {
			if(loops[i].end < last)
				return false;
			last = loops[i].end;
		}
		return (last == vertices.size());
	}

	void Clear() {
		vertices.clear();
		loops.clear();
	}
	void AddVertex(Vertex v) {
		vertices.push_back(v);
	}
	void AddLoopEnd(WindingNumber winding_weight) {
		loops.emplace_back(vertices.size(), winding_weight);
	}

	Vertex* GetLoopVertices(size_t i) {
		assert(i < loops.size());
		return (i == 0)? vertices.data() : vertices.data() + loops[i - 1].end;
	}
	const Vertex* GetLoopVertices(size_t i) const {
		assert(i < loops.size());
		return (i == 0)? vertices.data() : vertices.data() + loops[i - 1].end;
	}
	size_t GetLoopVertexCount(size_t i) const {
		assert(i < loops.size());
		return (i == 0)? loops[i].end : loops[i].end - loops[i - 1].end;
	}

	friend std::ostream& operator<<(std::ostream &stream, const Polygon &p) {
		if(p.IsValid()) {
			stream << "Polygon([";
			for(size_t i = 0; i < p.loops.size(); i++) {
				if(i != 0)
					stream << ", ";
				stream << "Loop([";
				const Vertex *vertices = p.GetLoopVertices(i);
				size_t n = p.GetLoopVertexCount(i);
				for(size_t j = 0; j < n; ++j) {
					if(j != 0)
						stream << ", ";
					stream << vertices[j];
				}
				stream << "], " << p.loops[i].weight << ")";
			}
			stream << "])";
		} else {
			stream << "Polygon(<invalid>)";
		}
		return stream;
	}

};

}
