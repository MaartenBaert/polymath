#pragma once

#include "Common.h"

#include "Polygon.h"
#include "Vertex.h"
#include "Visualization.h"

#include <memory>

namespace PolyMath {

template<typename T>
class OutputPolicy_Simple {

public:
	typedef T ValueType;
	typedef Vertex<T> VertexType;

private:
	struct OutputVertex {
		VertexType m_vertex;
		OutputVertex *m_next;
	};

public:
	struct OutputEdge {
		OutputVertex *m_output_vertex;
	};

public:
	static constexpr bool START_NEEDS_PREV_NEXT = false;
	static constexpr bool STOP_NEEDS_PREV_NEXT = false;

private:
	static constexpr size_t OUTPUT_VERTEX_BATCH_SIZE = 256;

private:
	std::vector<std::unique_ptr<OutputVertex[]>> m_output_vertex_batches;
	size_t m_output_vertex_batch_used;

private:
	OutputVertex* AddOutputVertex(VertexType vertex) {
		if(m_output_vertex_batch_used == OUTPUT_VERTEX_BATCH_SIZE) {
			std::unique_ptr<OutputVertex[]> mem(new OutputVertex[OUTPUT_VERTEX_BATCH_SIZE]);
			m_output_vertex_batches.push_back(std::move(mem));
			m_output_vertex_batch_used = 0;
		}
		OutputVertex *batch = m_output_vertex_batches.back().get();
		OutputVertex *v = &batch[m_output_vertex_batch_used];
		v->m_vertex = vertex;
		++m_output_vertex_batch_used;
		return v;
	}

public:
	OutputPolicy_Simple() {
		m_output_vertex_batch_used = OUTPUT_VERTEX_BATCH_SIZE;
	}

	static bool HasOutputEdge(OutputEdge &edge) {
		return (edge.m_output_vertex != nullptr);
	}

	static void ClearOutputEdge(OutputEdge &edge) {
		edge.m_output_vertex = nullptr;
	}

	static void CopyOutputEdge(OutputEdge &from, OutputEdge &to) {
		to.m_output_vertex = from.m_output_vertex;
	}

	static void SwapOutputEdges(OutputEdge &edge1, OutputEdge &edge2) {
		std::swap(edge1.m_output_vertex, edge2.m_output_vertex);
	}

	void OutputStartVertex(OutputEdge &edge1, OutputEdge &edge2, VertexType vertex, bool is_split, OutputEdge *edge_prev, OutputEdge *edge_next) {
		POLYMATH_UNUSED(edge_prev);
		POLYMATH_UNUSED(edge_next);

		// create new output vertex
		OutputVertex *output_vertex = AddOutputVertex(vertex);
		output_vertex->m_next = nullptr;

		// update edges
		edge1.m_output_vertex = output_vertex;
		edge2.m_output_vertex = output_vertex;

	}

	void OutputMiddleVertex(OutputEdge &edge, VertexType vertex, bool is_left) {
		assert(edge.m_output_vertex != nullptr);

		// create new output vertex
		OutputVertex *output_vertex = AddOutputVertex(vertex);

		// update edges
		if(is_left) {
			output_vertex->m_next = edge.m_output_vertex;
			edge.m_output_vertex = output_vertex;
		} else {
			output_vertex->m_next = nullptr;
			edge.m_output_vertex->m_next = output_vertex;
			edge.m_output_vertex = output_vertex;
		}

	}

	void OutputStopVertex(OutputEdge &edge1, OutputEdge &edge2, VertexType vertex, bool is_merge, OutputEdge *edge_prev, OutputEdge *edge_next) {
		POLYMATH_UNUSED(edge_prev);
		POLYMATH_UNUSED(edge_next);
		assert(edge1.m_output_vertex != nullptr);
		assert(edge2.m_output_vertex != nullptr);

		// create new output vertex
		OutputVertex *output_vertex = AddOutputVertex(vertex);

		// update edges
		if(is_merge) {
			edge1.m_output_vertex->m_next = output_vertex;
			output_vertex->m_next = edge2.m_output_vertex;
		} else {
			edge2.m_output_vertex->m_next = output_vertex;
			output_vertex->m_next = edge1.m_output_vertex;
		}

	}

	void Visualize(Visualization<T> &vis) {

		// output edges
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *batch = m_output_vertex_batches[i].get();
			size_t batch_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputVertex *v = &batch[j];
				if(v->m_next != nullptr) {
					vis.m_output_edges.emplace_back();
					auto &edge = vis.m_output_edges.back();
					edge.m_edge_vertices[0] = v->m_vertex;
					edge.m_edge_vertices[1] = v->m_next->m_vertex;
				}
			}
		}

	}

	template<typename W>
	Polygon<T, W> Result() {
		Polygon<T, W> result;

		// reserve space for all output vertices
		result.vertices.reserve(m_output_vertex_batches.size() * OUTPUT_VERTEX_BATCH_SIZE + m_output_vertex_batch_used - OUTPUT_VERTEX_BATCH_SIZE);

		// fill polygon with output vertex data
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *batch = m_output_vertex_batches[i].get();
			size_t batch_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputVertex *v = &batch[j];
				if(v->m_next != nullptr) {
					OutputVertex *w = v;
					while(w->m_next != nullptr) {
						result.AddVertex(w->m_vertex);
						OutputVertex *next = w->m_next;
						w->m_next = nullptr;
						w = next;
					}
					result.AddLoopEnd(1);
				}
			}
		}

		return result;
	}

};

template<typename T>
class OutputPolicy_Keyhole {

public:
	typedef T ValueType;
	typedef Vertex<T> VertexType;

private:
	struct OutputVertex {
		VertexType m_vertex;
		OutputVertex *m_next;
	};
	struct StartVertex {
		StartVertex *m_parent;
		size_t m_reference_count;
		OutputVertex *m_output_vertex;
	};

public:
	struct OutputEdge {
		OutputVertex *m_output_vertex;
		StartVertex *m_start_vertex, *m_cut_vertex;
	};

public:
	static constexpr bool START_NEEDS_PREV_NEXT = true;
	static constexpr bool STOP_NEEDS_PREV_NEXT = true;

private:
	static constexpr size_t OUTPUT_VERTEX_BATCH_SIZE = 256;
	static constexpr size_t START_VERTEX_BATCH_SIZE = 256;

private:
	std::vector<std::unique_ptr<OutputVertex[]>> m_output_vertex_batches;
	std::vector<std::unique_ptr<StartVertex[]>> m_start_vertex_batches;
	size_t m_output_vertex_batch_used, m_start_vertex_batch_used;

private:
	OutputVertex* AddOutputVertex(VertexType vertex) {
		if(m_output_vertex_batch_used == OUTPUT_VERTEX_BATCH_SIZE) {
			std::unique_ptr<OutputVertex[]> mem(new OutputVertex[OUTPUT_VERTEX_BATCH_SIZE]);
			m_output_vertex_batches.push_back(std::move(mem));
			m_output_vertex_batch_used = 0;
		}
		OutputVertex *batch = m_output_vertex_batches.back().get();
		OutputVertex *v = &batch[m_output_vertex_batch_used];
		v->m_vertex = vertex;
		++m_output_vertex_batch_used;
		return v;
	}

	StartVertex* AddStartVertex() {
		if(m_start_vertex_batch_used == START_VERTEX_BATCH_SIZE) {
			std::unique_ptr<StartVertex[]> mem(new StartVertex[START_VERTEX_BATCH_SIZE]);
			m_start_vertex_batches.push_back(std::move(mem));
			m_start_vertex_batch_used = 0;
		}
		StartVertex *batch = m_start_vertex_batches.back().get();
		StartVertex *v = &batch[m_start_vertex_batch_used];
		v->m_parent = nullptr;
		v->m_reference_count = 1;
		++m_start_vertex_batch_used;
		return v;
	}

	StartVertex *FindStartVertexRoot(StartVertex *vertex) {
		assert(vertex != nullptr);
		if(vertex->m_parent == nullptr)
			return vertex;
		StartVertex *root = FindStartVertexRoot(vertex->m_parent);
		vertex->m_parent = root;
		return root;
	}

public:
	OutputPolicy_Keyhole() {
		m_output_vertex_batch_used = OUTPUT_VERTEX_BATCH_SIZE;
		m_start_vertex_batch_used = START_VERTEX_BATCH_SIZE;
	}

	static bool HasOutputEdge(OutputEdge &edge) {
		return (edge.m_output_vertex != nullptr);
	}

	static void ClearOutputEdge(OutputEdge &edge) {
		edge.m_output_vertex = nullptr;
	}

	static void CopyOutputEdge(OutputEdge &from, OutputEdge &to) {
		to.m_output_vertex = from.m_output_vertex;
		to.m_cut_vertex = from.m_cut_vertex;
		to.m_start_vertex = from.m_start_vertex;
	}

	static void SwapOutputEdges(OutputEdge &edge1, OutputEdge &edge2) {
		std::swap(edge1.m_output_vertex, edge2.m_output_vertex);
		std::swap(edge1.m_cut_vertex, edge2.m_cut_vertex);
		std::swap(edge1.m_start_vertex, edge2.m_start_vertex);
	}

	void OutputStartVertex(OutputEdge &edge1, OutputEdge &edge2, VertexType vertex, bool is_split, OutputEdge *edge_prev, OutputEdge *edge_next) {

		if(is_split && edge_prev->m_cut_vertex != nullptr && edge_prev->m_cut_vertex->m_parent == nullptr) {
			assert(edge_prev != nullptr);
			assert(edge_next != nullptr);
			assert(edge_prev->m_cut_vertex->m_reference_count == 0);

			// reuse existing start vertex
			StartVertex *start_vertex = edge_prev->m_cut_vertex;
			start_vertex->m_reference_count = 1;
			edge_prev->m_cut_vertex = nullptr;
			edge_next->m_cut_vertex = nullptr;

			// create two output vertices
			OutputVertex *output_vertex1 = AddOutputVertex(vertex);
			OutputVertex *output_vertex2 = AddOutputVertex(vertex);
			output_vertex1->m_next = nullptr;
			output_vertex2->m_next = start_vertex->m_output_vertex->m_next;
			start_vertex->m_output_vertex->m_next = output_vertex1;

			// update edges
			edge1.m_output_vertex = output_vertex1;
			edge1.m_cut_vertex = nullptr;
			edge1.m_start_vertex = start_vertex;
			edge2.m_output_vertex = output_vertex2;
			edge2.m_cut_vertex = nullptr;
			edge2.m_start_vertex = start_vertex;

		} else {

			// create new start vertex
			StartVertex *start_vertex = AddStartVertex();

			// create new output vertex
			OutputVertex *output_vertex = AddOutputVertex(vertex);
			output_vertex->m_next = nullptr;

			// update edges
			edge1.m_output_vertex = output_vertex;
			edge1.m_cut_vertex = nullptr;
			edge1.m_start_vertex = start_vertex;
			edge2.m_output_vertex = output_vertex;
			edge2.m_cut_vertex = nullptr;
			edge2.m_start_vertex = start_vertex;

		}

	}

	void OutputMiddleVertex(OutputEdge &edge, VertexType vertex, bool is_left) {
		assert(edge.m_output_vertex != nullptr);

		if(edge.m_cut_vertex != nullptr && edge.m_cut_vertex->m_parent == nullptr) {
			assert(edge.m_cut_vertex->m_reference_count == 0);

			// join roots
			edge.m_cut_vertex->m_parent = edge.m_start_vertex;

			// create two new output vertices
			OutputVertex *output_vertex1 = AddOutputVertex(vertex);
			OutputVertex *output_vertex2 = AddOutputVertex(vertex);
			output_vertex2->m_next = edge.m_cut_vertex->m_output_vertex->m_next;
			edge.m_cut_vertex->m_output_vertex->m_next = output_vertex1;

			// update edges
			if(is_left) {
				output_vertex1->m_next = edge.m_output_vertex;
				edge.m_output_vertex = output_vertex2;
			} else {
				output_vertex1->m_next = nullptr;
				edge.m_output_vertex->m_next = output_vertex2;
				edge.m_output_vertex = output_vertex1;
			}

		} else {

			// create new output vertex
			OutputVertex *output_vertex = AddOutputVertex(vertex);

			// update edges
			if(is_left) {
				output_vertex->m_next = edge.m_output_vertex;
				edge.m_output_vertex = output_vertex;
			} else {
				output_vertex->m_next = nullptr;
				edge.m_output_vertex->m_next = output_vertex;
				edge.m_output_vertex = output_vertex;
			}

		}

		// clear cut
		edge.m_cut_vertex = nullptr;

	}

	void OutputStopVertex(OutputEdge &edge1, OutputEdge &edge2, VertexType vertex, bool is_merge, OutputEdge *edge_prev, OutputEdge *edge_next) {
		assert(edge1.m_output_vertex != nullptr);
		assert(edge2.m_output_vertex != nullptr);

		// join roots if necessary
		StartVertex *root1 = FindStartVertexRoot(edge1.m_start_vertex);
		StartVertex *root2 = FindStartVertexRoot(edge2.m_start_vertex);
		if(root1 != root2) {
			if(root1->m_reference_count < root2->m_reference_count) {
				std::swap(root1, root2);
			}
			root2->m_parent = root1;
			root1->m_reference_count += root2->m_reference_count;
		}
		--root1->m_reference_count;

		if(is_merge) {

			OutputVertex *edge1_vertex;
			if(edge1.m_cut_vertex != nullptr && edge1.m_cut_vertex->m_parent == nullptr) {
				assert(edge1.m_cut_vertex->m_reference_count == 0);

				edge1_vertex = edge1.m_cut_vertex->m_output_vertex;

				// join roots
				edge1.m_cut_vertex->m_parent = root1;

				// create new output vertex
				OutputVertex *output_vertex = AddOutputVertex(vertex);
				output_vertex->m_next = edge1.m_cut_vertex->m_output_vertex->m_next;
				edge1.m_output_vertex->m_next = output_vertex;

			} else {
				edge1_vertex = edge1.m_output_vertex;
			}

			OutputVertex *edge2_vertex;
			if(edge2.m_cut_vertex != nullptr && edge2.m_cut_vertex->m_parent == nullptr) {
				assert(edge2.m_cut_vertex->m_reference_count == 0);

				edge2_vertex = edge2.m_cut_vertex->m_output_vertex->m_next;

				// join roots
				edge2.m_cut_vertex->m_parent = root1;

				// create new output vertex
				OutputVertex *output_vertex = AddOutputVertex(vertex);
				output_vertex->m_next = edge2.m_output_vertex;
				edge2.m_cut_vertex->m_output_vertex->m_next = output_vertex;

			} else {
				edge2_vertex = edge2.m_output_vertex;
			}

			if(root1->m_reference_count == 0) {
				assert(edge_prev != nullptr);
				assert(edge_next != nullptr);

				// create two new output vertices
				OutputVertex *output_vertex1 = AddOutputVertex(vertex);
				OutputVertex *output_vertex2 = AddOutputVertex(vertex);
				edge1_vertex->m_next = output_vertex1;
				output_vertex1->m_next = output_vertex2;
				output_vertex2->m_next = edge2_vertex;

				// trigger future cut
				root1->m_output_vertex = output_vertex1;
				edge_prev->m_cut_vertex = root1;
				edge_next->m_cut_vertex = root1;

			} else {

				// create new output vertex
				OutputVertex *output_vertex = AddOutputVertex(vertex);

				// close loop
				edge1_vertex->m_next = output_vertex;
				output_vertex->m_next = edge2_vertex;

			}
		} else {

			if(edge1.m_cut_vertex != nullptr && edge1.m_cut_vertex->m_parent == nullptr) {
				assert(edge1.m_cut_vertex->m_reference_count == 0);

				// join roots
				edge1.m_cut_vertex->m_parent = root1;

				// create two new output vertices
				OutputVertex *output_vertex1 = AddOutputVertex(vertex);
				OutputVertex *output_vertex2 = AddOutputVertex(vertex);
				output_vertex1->m_next = edge1.m_output_vertex;
				output_vertex2->m_next = edge1.m_cut_vertex->m_output_vertex->m_next;
				edge1.m_cut_vertex->m_output_vertex->m_next = output_vertex1;
				edge2.m_output_vertex->m_next = output_vertex2;

			} else {

				// create new output vertex
				OutputVertex *output_vertex = AddOutputVertex(vertex);

				// close loop
				edge2.m_output_vertex->m_next = output_vertex;
				output_vertex->m_next = edge1.m_output_vertex;

			}

		}

	}

	void Visualize(Visualization<T> &vis) {

		// output edges
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *batch = m_output_vertex_batches[i].get();
			size_t batch_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputVertex *v = &batch[j];
				if(v->m_next != nullptr) {
					vis.m_output_edges.emplace_back();
					auto &edge = vis.m_output_edges.back();
					edge.m_edge_vertices[0] = v->m_vertex;
					edge.m_edge_vertices[1] = v->m_next->m_vertex;
				}
			}
		}

	}

	template<typename W>
	Polygon<T, W> Result() {
		Polygon<T, W> result;

		// reserve space for all output vertices
		result.vertices.reserve(m_output_vertex_batches.size() * OUTPUT_VERTEX_BATCH_SIZE + m_output_vertex_batch_used - OUTPUT_VERTEX_BATCH_SIZE);

		// fill polygon with output vertex data
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *batch = m_output_vertex_batches[i].get();
			size_t batch_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputVertex *v = &batch[j];
				if(v->m_next != nullptr) {
					OutputVertex *w = v;
					while(w->m_next != nullptr) {
						result.AddVertex(w->m_vertex);
						OutputVertex *next = w->m_next;
						w->m_next = nullptr;
						w = next;
					}
					result.AddLoopEnd(1);
				}
			}
		}

		return result;
	}

};

template<typename T>
class OutputPolicy_Monotone {

public:
	typedef T ValueType;
	typedef Vertex<T> VertexType;

private:
	struct OutputVertex {
		VertexType m_vertex;
		OutputVertex *m_next;
	};
	struct OutputPolygon {
		OutputPolygon *m_opponent;
		OutputVertex *m_chain1, *m_chain2;
		bool m_last_forward;
		VertexType m_stop_vertex;
	};

public:
	struct OutputEdge {
		OutputPolygon *m_output_polygon;
		bool m_output_forward;
	};

public:
	static constexpr bool START_NEEDS_PREV_NEXT = true;
	static constexpr bool STOP_NEEDS_PREV_NEXT = false;

private:
	static constexpr size_t OUTPUT_VERTEX_BATCH_SIZE = 256;
	static constexpr size_t OUTPUT_POLYGON_BATCH_SIZE = 256;

private:
	std::vector<std::unique_ptr<OutputVertex[]>> m_output_vertex_batches;
	std::vector<std::unique_ptr<OutputPolygon[]>> m_output_polygon_batches;
	size_t m_output_vertex_batch_used, m_output_polygon_batch_used;

private:
	OutputVertex* AddOutputVertex(VertexType vertex) {
		if(m_output_vertex_batch_used == OUTPUT_VERTEX_BATCH_SIZE) {
			std::unique_ptr<OutputVertex[]> mem(new OutputVertex[OUTPUT_VERTEX_BATCH_SIZE]);
			m_output_vertex_batches.push_back(std::move(mem));
			m_output_vertex_batch_used = 0;
		}
		OutputVertex *batch = m_output_vertex_batches.back().get();
		OutputVertex *v = &batch[m_output_vertex_batch_used];
		v->m_vertex = vertex;
		++m_output_vertex_batch_used;
		return v;
	}

	OutputPolygon* AddOutputPolygon() {
		if(m_output_polygon_batch_used == OUTPUT_POLYGON_BATCH_SIZE) {
			std::unique_ptr<OutputPolygon[]> mem(new OutputPolygon[OUTPUT_POLYGON_BATCH_SIZE]);
			m_output_polygon_batches.push_back(std::move(mem));
			m_output_polygon_batch_used = 0;
		}
		OutputPolygon *batch = m_output_polygon_batches.back().get();
		OutputPolygon *v = &batch[m_output_polygon_batch_used];
		++m_output_polygon_batch_used;
		return v;
	}

public:
	OutputPolicy_Monotone() {
		m_output_vertex_batch_used = OUTPUT_VERTEX_BATCH_SIZE;
		m_output_polygon_batch_used = OUTPUT_POLYGON_BATCH_SIZE;
	}

	static bool HasOutputEdge(OutputEdge &edge) {
		return (edge.m_output_polygon != nullptr);
	}

	static void ClearOutputEdge(OutputEdge &edge) {
		edge.m_output_polygon = nullptr;
	}

	static void CopyOutputEdge(OutputEdge &from, OutputEdge &to) {
		to.m_output_polygon = from.m_output_polygon;
		to.m_output_forward = from.m_output_forward;
	}

	static void SwapOutputEdges(OutputEdge &edge1, OutputEdge &edge2) {
		std::swap(edge1.m_output_polygon, edge2.m_output_polygon);
		std::swap(edge1.m_output_forward, edge2.m_output_forward);
	}

	void OutputStartVertex(OutputEdge &edge1, OutputEdge &edge2, VertexType vertex, bool is_split, OutputEdge *edge_prev, OutputEdge *edge_next) {

		if(is_split) {

			// get existing output polygon
			OutputPolygon *output_polygon1 = edge_prev->m_output_polygon;
			OutputPolygon *output_polygon2 = output_polygon1->m_opponent; //edge_next->m_output_polygon;
			if(output_polygon2 == nullptr) {

				if(output_polygon1->m_last_forward) {

					// create new output vertices
					OutputVertex *output_vertex0 = AddOutputVertex(output_polygon1->m_chain2->m_vertex);
					output_vertex0->m_next = nullptr;
					OutputVertex *output_vertex1 = AddOutputVertex(vertex);
					output_vertex1->m_next = output_polygon1->m_chain2;
					OutputVertex *output_vertex2 = AddOutputVertex(vertex);
					output_vertex2->m_next = output_vertex0;

					// update output polygon
					output_polygon1->m_chain2 = output_vertex1;
					output_polygon1->m_last_forward = true;

					// create new output polygon
					OutputPolygon *output_polygon = AddOutputPolygon();
					output_polygon->m_opponent = nullptr;
					output_polygon->m_chain1 = output_vertex2;
					output_polygon->m_chain2 = output_vertex0;
					output_polygon->m_last_forward = false;

					// update edges
					edge1.m_output_polygon = output_polygon1;
					edge1.m_output_forward = true;
					edge2.m_output_polygon = output_polygon;
					edge2.m_output_forward = false;
					edge_next->m_output_polygon = output_polygon;

				} else {

					// create new output vertices
					OutputVertex *output_vertex0 = AddOutputVertex(output_polygon1->m_chain1->m_vertex);
					output_vertex0->m_next = nullptr;
					OutputVertex *output_vertex1 = AddOutputVertex(vertex);
					output_vertex1->m_next = output_vertex0;
					OutputVertex *output_vertex2 = AddOutputVertex(vertex);
					output_vertex2->m_next = output_polygon1->m_chain1;

					// update output polygon
					output_polygon1->m_chain1 = output_vertex2;
					output_polygon1->m_last_forward = false;

					// create new output polygon
					OutputPolygon *output_polygon = AddOutputPolygon();
					output_polygon->m_opponent = nullptr;
					output_polygon->m_chain1 = output_vertex0;
					output_polygon->m_chain2 = output_vertex1;
					output_polygon->m_last_forward = true;

					// update edges
					edge1.m_output_polygon = output_polygon;
					edge1.m_output_forward = true;
					edge2.m_output_polygon = output_polygon1;
					edge2.m_output_forward = false;
					edge_prev->m_output_polygon = output_polygon;

				}

			} else {

				// create new output vertices
				OutputVertex *output_vertex1 = AddOutputVertex(vertex);
				output_vertex1->m_next = output_polygon1->m_chain2;
				OutputVertex *output_vertex2 = AddOutputVertex(vertex);
				output_vertex2->m_next = output_polygon2->m_chain1;

				// update output polygons
				output_polygon1->m_opponent = nullptr;
				output_polygon1->m_chain2 = output_vertex1;
				output_polygon1->m_last_forward = true;
				output_polygon2->m_opponent = nullptr;
				output_polygon2->m_chain1 = output_vertex2;
				output_polygon2->m_last_forward = false;

				// update edges
				edge1.m_output_polygon = output_polygon1;
				edge1.m_output_forward = true;
				edge2.m_output_polygon = output_polygon2;
				edge2.m_output_forward = false;

			}

		} else {

			// create new output vertex
			OutputVertex *output_vertex = AddOutputVertex(vertex);
			output_vertex->m_next = nullptr;

			// create new output polygon
			OutputPolygon *output_polygon = AddOutputPolygon();
			output_polygon->m_opponent = nullptr;
			output_polygon->m_chain1 = output_vertex;
			output_polygon->m_chain2 = output_vertex;
			output_polygon->m_last_forward = false;

			// update edges
			edge1.m_output_polygon = output_polygon;
			edge1.m_output_forward = is_split;
			edge2.m_output_polygon = output_polygon;
			edge2.m_output_forward = !is_split;

		}

	}

	void OutputMiddleVertex(OutputEdge &edge, VertexType vertex, bool is_left) {
		assert(edge.m_output_polygon != nullptr);

		if(edge.m_output_forward) {

			// deal with opponent
			if(edge.m_output_polygon->m_opponent != nullptr) {
				edge.m_output_polygon->m_stop_vertex = vertex;
				edge.m_output_polygon = edge.m_output_polygon->m_opponent;
				edge.m_output_polygon->m_opponent = nullptr;
			}

			// create new output vertex
			OutputVertex *output_vertex = AddOutputVertex(vertex);
			output_vertex->m_next = edge.m_output_polygon->m_chain2;

			// update output polygons
			edge.m_output_polygon->m_chain2 = output_vertex;
			edge.m_output_polygon->m_last_forward = true;

		} else {

			// deal with opponent
			if(edge.m_output_polygon->m_opponent != nullptr) {
				edge.m_output_polygon->m_stop_vertex = vertex;
				edge.m_output_polygon = edge.m_output_polygon->m_opponent;
				edge.m_output_polygon->m_opponent = nullptr;
			}

			// create new output vertex
			OutputVertex *output_vertex = AddOutputVertex(vertex);
			output_vertex->m_next = edge.m_output_polygon->m_chain1;

			// update output polygons
			edge.m_output_polygon->m_chain1 = output_vertex;
			edge.m_output_polygon->m_last_forward = false;

		}

	}

	void OutputStopVertex(OutputEdge &edge1, OutputEdge &edge2, VertexType vertex, bool is_merge, OutputEdge *edge_prev, OutputEdge *edge_next) {
		POLYMATH_UNUSED(edge_prev);
		POLYMATH_UNUSED(edge_next);
		assert(edge1.m_output_polygon != nullptr);
		assert(edge2.m_output_polygon != nullptr);

		if(edge1.m_output_forward) {

			// deal with opponents
			if(edge1.m_output_polygon->m_opponent != nullptr) {
				edge1.m_output_polygon->m_stop_vertex = vertex;
				edge1.m_output_polygon = edge1.m_output_polygon->m_opponent;
			}
			if(edge2.m_output_polygon->m_opponent != nullptr) {
				edge2.m_output_polygon->m_stop_vertex = vertex;
				edge2.m_output_polygon = edge2.m_output_polygon->m_opponent;
			}

			// create new output vertices
			OutputVertex *output_vertex1 = AddOutputVertex(vertex);
			output_vertex1->m_next = edge1.m_output_polygon->m_chain2;
			OutputVertex *output_vertex2 = AddOutputVertex(vertex);
			output_vertex2->m_next = edge2.m_output_polygon->m_chain1;

			// update output polygon
			edge1.m_output_polygon->m_opponent = edge2.m_output_polygon;
			edge1.m_output_polygon->m_chain2 = output_vertex1;
			edge1.m_output_polygon->m_last_forward = true;
			edge2.m_output_polygon->m_opponent = edge1.m_output_polygon;
			edge2.m_output_polygon->m_chain1 = output_vertex2;
			edge2.m_output_polygon->m_last_forward = false;

		} else {

			if(edge1.m_output_polygon == edge2.m_output_polygon) {

				assert(edge1.m_output_polygon->m_opponent == nullptr);
				assert(edge2.m_output_polygon->m_opponent == nullptr);

				edge1.m_output_polygon->m_stop_vertex = vertex;

			} else {

				assert(edge1.m_output_polygon->m_opponent == edge2.m_output_polygon);
				assert(edge2.m_output_polygon->m_opponent == edge1.m_output_polygon);

				edge1.m_output_polygon->m_stop_vertex = vertex;
				edge2.m_output_polygon->m_stop_vertex = vertex;

			}

		}

	}

	void Visualize(Visualization<T> &vis) {

		// output edges
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *batch = m_output_vertex_batches[i].get();
			size_t batch_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputVertex *v = &batch[j];
				if(v->m_next != nullptr) {
					vis.m_output_edges.emplace_back();
					auto &edge = vis.m_output_edges.back();
					edge.m_edge_vertices[0] = v->m_vertex;
					edge.m_edge_vertices[1] = v->m_next->m_vertex;
				}
			}
		}

	}

	template<typename W>
	Polygon<T, W> Result() {
		Polygon<T, W> result;

		// reserve space for all output vertices
		result.vertices.reserve(m_output_vertex_batches.size() * OUTPUT_VERTEX_BATCH_SIZE + m_output_vertex_batch_used - OUTPUT_VERTEX_BATCH_SIZE);

		// fill polygon with output vertex data
		for(size_t i = 0; i < m_output_polygon_batches.size(); ++i) {
			OutputPolygon *batch = m_output_polygon_batches[i].get();
			size_t batch_size = (i == m_output_polygon_batches.size() - 1)? m_output_polygon_batch_used : OUTPUT_POLYGON_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputPolygon *p = &batch[j];
				result.AddVertex(p->m_stop_vertex);
				OutputVertex *v = p->m_chain1;
				while(v != nullptr) {
					result.AddVertex(v->m_vertex);
					v = v->m_next;
				}
				OutputVertex *w = p->m_chain2;
				size_t rev = 0;
				while(w->m_next != nullptr) {
					result.AddVertex(w->m_vertex);
					w = w->m_next;
					++rev;
				}
				std::reverse(result.vertices.end() - rev, result.vertices.end());
				result.AddLoopEnd(1);
			}
		}

		return result;
	}

};

}
