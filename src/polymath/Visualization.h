#pragma once

#include "Common.h"

namespace PolyMath {

template<class Vertex>
struct Visualization {

	typedef typename Vertex::value_type value_type;

	struct SweepEdge {
		Vertex m_edge_vertices[2];
		bool m_has_intersection;
		Vertex m_intersection_vertex;
		int64_t m_winding_number;
	};

	struct OutputEdge {
		Vertex m_edge_vertices[2];
	};

	bool m_has_current_vertex;
	Vertex m_current_vertex;
	std::vector<SweepEdge> m_sweep_edges;
	std::vector<OutputEdge> m_output_edges;

	Visualization() = default;
	Visualization(const Visualization &other) = default;
	Visualization(Visualization &&other) = default;

	Visualization& operator=(const Visualization &other) = default;
	Visualization& operator=(Visualization &&other) = default;

};

}
