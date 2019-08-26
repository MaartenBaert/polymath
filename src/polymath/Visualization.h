#pragma once

#include "Common.h"

#include "Vertex.h"

namespace PolyMath {

template<typename T>
struct Visualization {

	typedef Vertex<T> VertexType;

	struct SweepEdge {
		VertexType m_edge_vertices[2];
		bool m_has_intersection;
		VertexType m_intersection_vertex;
		int64_t m_winding_number;
	};

	struct OutputEdge {
		VertexType m_edge_vertices[2];
	};

	bool m_has_current_vertex;
	VertexType m_current_vertex;
	std::vector<SweepEdge> m_sweep_edges;
	std::vector<OutputEdge> m_output_edges;

	Visualization() = default;
	Visualization(const Visualization &other) = default;
	Visualization(Visualization &&other) = default;

	Visualization& operator=(const Visualization &other) = default;
	Visualization& operator=(Visualization &&other) = default;

};

}
