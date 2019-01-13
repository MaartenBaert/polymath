#pragma once

#include "Common.h"

#include "NumericalEngine.h"
#include "Polygon.h"
#include "Vertex.h"
#include "Visualization.h"

#include <algorithm>
#include <limits>
#include <memory>

#define POLYMATH_VERIFY 0

#if POLYMATH_VERIFY
#include <iostream>
#endif

// TODO: remove unnecessary initializations
// - tree (?)
// - heap index (requires changes)
// - output vertex

// TODO: replace vector with unique_ptr?

namespace PolyMath {

inline void DummyVisualizationCallback() {
	// nothing
}

template<class Vertex, class WindingEngine>
class SweepEngine {

public:
	typedef PolyMath::NumericalEngine<typename Vertex::ValueType> NumericalEngine;
	typedef typename NumericalEngine::SingleType SingleType;
	typedef typename NumericalEngine::DoubleType DoubleType;
	typedef PolyMath::Vertex<SingleType> SingleVertex;
	typedef PolyMath::Vertex<DoubleType> DoubleVertex;

private:
	struct SweepEdge;

	struct OutputVertex {
		Vertex m_vertex;
		OutputVertex *m_next;
	};

	struct SweepVertex {

		// vertex data
		Vertex m_vertex;
		int64_t m_winding_weight;

		// loop
		SweepVertex *m_loop_prev, *m_loop_next;
		bool m_edge_forward;

		// sweep edge
		SweepEdge *m_sweep_edge;

	};

	struct SweepEdge {

		// vertices
		Vertex m_vertex_first, m_vertex_last;

		// tree
		SweepEdge *m_tree_parent, *m_tree_left, *m_tree_right;
		bool m_tree_red;

		// heap
		DoubleVertex m_heap_vertex;
		size_t m_heap_index;

		// winding number
		int64_t m_winding_weight, m_winding_number;

		// output
		OutputVertex *m_output_vertex;
		bool m_output_vertex_forward;

	};

private:
	static constexpr size_t SWEEP_EDGE_BATCH_SIZE = 256;
	static constexpr size_t OUTPUT_VERTEX_BATCH_SIZE = 256;

private:

	// input vertices
	std::vector<SweepVertex> m_vertex_pool;

	// sorted vertices
	std::vector<SweepVertex*> m_vertex_queue;
	size_t m_current_vertex;

	// sweep edges
	std::vector<std::unique_ptr<SweepEdge[]>> m_sweep_edge_batches;
	SweepEdge *m_sweep_edge_free_list;

	// tree (edges intersecting sweepline)
	SweepEdge *m_tree_root;

	// heap (intersections)
	std::vector<SweepEdge*> m_heap;

	// output vertices
	std::vector<std::unique_ptr<OutputVertex[]>> m_output_vertex_batches;
	size_t m_output_vertex_batch_used;

private:

	// The 'less than' operator for vertices. It returns whether a comes before b.
	static bool CompareVertexVertex(SweepVertex *a, SweepVertex *b) {
		// compare the X coordinates first
		if(a->m_vertex.x < b->m_vertex.x)
			return true;
		if(a->m_vertex.x > b->m_vertex.x)
			return false;
		// if the X coordinates are equal, compare the Y coordinates (this simplifies a lot of edge cases)
		if(a->m_vertex.y < b->m_vertex.y)
			return true;
		if(a->m_vertex.y > b->m_vertex.y)
			return false;
		// The tie breaker can be anything, as long as it's consistent. The pointer value is unique,
		// and in this case it's even deterministic because all vertices are in the same array.
		return (a < b);
	}

	// The 'less than' operator for an active edge and a vertex. This is used to insert new points in the search tree.
	static bool CompareEdgeVertex(SweepEdge *edge, SweepVertex *vertex) {

		// get points
		SingleType a1_x = edge->m_vertex_first.x;
		SingleType a1_y = edge->m_vertex_first.y;
		SingleType a2_x = edge->m_vertex_last.x;
		SingleType a2_y = edge->m_vertex_last.y;
		SingleType b_x = vertex->m_vertex.x;
		SingleType b_y = vertex->m_vertex.y;
		assert(a1_x <= b_x);
		assert(b_x <= a2_x);

		// test
		return NumericalEngine::OrientationTest(a1_x, a1_y, a2_x, a2_y, b_x, b_y, true);

	}

	// Calculates the intersection between two edges.
	static bool IntersectEdgeEdge(SweepEdge *edge1, SweepEdge *edge2, DoubleVertex &result) {

		// get points
		SingleType a1_x = edge1->m_vertex_first.x;
		SingleType a1_y = edge1->m_vertex_first.y;
		SingleType a2_x = edge1->m_vertex_last.x;
		SingleType a2_y = edge1->m_vertex_last.y;
		SingleType b1_x = edge2->m_vertex_first.x;
		SingleType b1_y = edge2->m_vertex_first.y;
		SingleType b2_x = edge2->m_vertex_last.x;
		SingleType b2_y = edge2->m_vertex_last.y;
		assert(a1_x <= a2_x);
		assert(a1_x <= b2_x);
		assert(b1_x <= a2_x);
		assert(b1_x <= b2_x);

		// test
		DoubleType res_x, res_y;
		if(NumericalEngine::IntersectionTest(a1_x, a1_y, a2_x, a2_y, b1_x, b1_y, b2_x, b2_y, res_x, res_y)) {
			result = DoubleVertex(res_x, res_y);
			return true;
		}
		return false;

	}

	SweepEdge* AddSweepEdge() {

		if(m_sweep_edge_free_list == nullptr) {

			// allocate block
			std::unique_ptr<SweepEdge[]> mem(new SweepEdge[SWEEP_EDGE_BATCH_SIZE]);
			SweepEdge *edges = mem.get();
			m_sweep_edge_batches.push_back(std::move(mem));

			// build linked list
			for(size_t i = 0; i < SWEEP_EDGE_BATCH_SIZE; ++i) {
				edges[i].m_tree_parent = (i == SWEEP_EDGE_BATCH_SIZE - 1)? nullptr : &edges[i + 1];
			}

			// add to free list
			m_sweep_edge_free_list = &edges[0];

		}

		// remove edge from free list
		SweepEdge *edge = m_sweep_edge_free_list;
		m_sweep_edge_free_list = edge->m_tree_parent;

		// initialize binary search tree
		/*edge->m_tree_parent = nullptr;
		edge->m_tree_left = nullptr;
		edge->m_tree_right = nullptr;
		edge->m_tree_red = false;*/

		// initialize heap
		edge->m_heap_index = INDEX_NONE;

		// initialize winding number
		/*edge->m_winding_number = 0;*/

		// initialize output
		/*edge->m_output_vertex = nullptr;
		edge->m_output_vertex_forward = false;*/

		return edge;

	}

	void RemoveSweepEdge(SweepEdge *edge) {

		// add edge to free list
		edge->m_tree_parent = m_sweep_edge_free_list;
		m_sweep_edge_free_list = edge;

	}

#if POLYMATH_VERIFY

	static void TreePrintEdge(SweepEdge *edge, size_t depth) {
		if(edge != nullptr) {
			TreePrintEdge(edge->m_tree_left, depth + 1);
		}
		for(size_t i = 0; i < depth; ++i) {
			std::cout << "  ";
		}
		if(edge == nullptr) {
			//std::cout << "." << std::endl;
		} else {
			if(edge->m_tree_red)
				std::cout << "R ";
			else
				std::cout << "- ";
			std::cout << edge->m_vertex_first << " " << edge->m_vertex_last << " <" << edge->m_winding_number << ">" << std::endl;
			TreePrintEdge(edge->m_tree_right, depth + 1);
		}
	}

	void TreePrint() {
		TreePrintEdge(m_tree_root, 0);
		std::cout << std::endl;
	}

	static size_t TreeVerifyEdge(SweepEdge *edge) {

		// basic checks
		if(edge == nullptr)
			return 1;

		// if the childs are not leafs, verify the parent pointer
		// we would like to verify the order as well (i.e. left < v and v < right),
		// but that may not work due to rounding errors and degenerate cases
		assert(edge->m_tree_left == nullptr || edge->m_tree_left->m_tree_parent == edge);
		assert(edge->m_tree_right == nullptr || edge->m_tree_right->m_tree_parent == edge);

		// verify subtrees
		size_t black_left = TreeVerifyEdge(edge->m_tree_left);
		size_t black_right = TreeVerifyEdge(edge->m_tree_right);
		assert(black_left == black_right);

		// verify the red-black rule
		if(edge->m_tree_red) {
			assert(edge->m_tree_left == nullptr || !edge->m_tree_left->m_tree_red);
			assert(edge->m_tree_right == nullptr || !edge->m_tree_right->m_tree_red);
			return black_left;
		} else {
			return black_left + 1;
		}

	}

	void TreeVerify() {

		// basic root checks
		assert(m_tree_root == nullptr || !m_tree_root->m_tree_red);

		// verify tree
		TreeVerifyEdge(m_tree_root);

	}

#endif

	void TreeSwap(SweepEdge *edge1, SweepEdge *edge2) {
		assert(edge1 != nullptr);
		assert(edge2 != nullptr);

		// swapping two edges is harder than it sounds because we need to handle all possible edge cases
		// copy original pointers to avoid confusion
		SweepEdge *p1 = edge1->m_tree_parent, *p2 = edge2->m_tree_parent;
		SweepEdge *l1 = edge1->m_tree_left, *l2 = edge2->m_tree_left;
		SweepEdge *r1 = edge1->m_tree_right, *r2 = edge2->m_tree_right;

		// swap colors
		std::swap(edge1->m_tree_red, edge2->m_tree_red);

		// fix internal pointers
		edge1->m_tree_parent = (p2 == edge1)? edge2 : p2;
		edge1->m_tree_left = (l2 == edge1)? edge2 : l2;
		edge1->m_tree_right = (r2 == edge1)? edge2 : r2;
		edge2->m_tree_parent = (p1 == edge2)? edge1 : p1;
		edge2->m_tree_left = (l1 == edge2)? edge1 : l1;
		edge2->m_tree_right = (r1 == edge2)? edge1 : r1;

		// fix pointers from parents
		if(p1 != edge2) {
			if(p1 == nullptr) {
				m_tree_root = edge2;
			} else if(p1->m_tree_left == edge1) {
				p1->m_tree_left = edge2;
			} else {
				p1->m_tree_right = edge2;
			}
		}
		if(p2 != edge1) {
			if(p2 == nullptr) {
				m_tree_root = edge1;
			} else if(p2->m_tree_left == edge2) {
				p2->m_tree_left = edge1;
			} else {
				p2->m_tree_right = edge1;
			}
		}

		// fix pointers from children
		if(l1 != nullptr && l1 != edge2)
			l1->m_tree_parent = edge2;
		if(r1 != nullptr && r1 != edge2)
			r1->m_tree_parent = edge2;
		if(l2 != nullptr && l2 != edge1)
			l2->m_tree_parent = edge1;
		if(r2 != nullptr && r2 != edge1)
			r2->m_tree_parent = edge1;

#if POLYMATH_VERIFY
		//TreePrint();
		TreeVerify();
#endif

	}

	// tree helper functions
	SweepEdge* TreeRotateLeft(SweepEdge *head) {
		assert(head != nullptr);
		assert(head->m_tree_right != nullptr);

		//  ,-H-,   =>   ,-A-,
		//  x  ,A,  =>  ,H,  x
		//     b x  =>  x b
		SweepEdge *parent = head->m_tree_parent, *a = head->m_tree_right, *b = a->m_tree_left;
		if(parent == nullptr) {
			m_tree_root = a;
			a->m_tree_parent = nullptr;
		} else if(parent->m_tree_left == head) {
			parent->m_tree_left = a;
			a->m_tree_parent = parent;
		} else {
			parent->m_tree_right = a;
			a->m_tree_parent = parent;
		}
		head->m_tree_right = b;
		if(b != nullptr)
			b->m_tree_parent = head;
		a->m_tree_left = head;
		head->m_tree_parent = a;

		// return new head
		return a;

	}

	SweepEdge* TreeRotateRight(SweepEdge *head) {
		assert(head != nullptr);
		assert(head->m_tree_left != nullptr);

		//   ,-H-,  =>  ,-A-,
		//  ,A,  x  =>  x  ,H,
		//  x b     =>     b x
		SweepEdge *parent = head->m_tree_parent, *a = head->m_tree_left, *b = a->m_tree_right;
		if(parent == nullptr) {
			m_tree_root = a;
			a->m_tree_parent = nullptr;
		} else if(parent->m_tree_left == head) {
			parent->m_tree_left = a;
			a->m_tree_parent = parent;
		} else {
			parent->m_tree_right = a;
			a->m_tree_parent = parent;
		}
		head->m_tree_left = b;
		if(b != nullptr)
			b->m_tree_parent = head;
		a->m_tree_right = head;
		head->m_tree_parent = a;

		// return new head
		return a;

	}

	void TreeInsert(SweepEdge *edge, SweepVertex *vertex, SweepEdge *after = nullptr) {
		assert(edge != nullptr);
		assert(vertex != nullptr);

		// insert edge
		if(after == nullptr) {
			if(m_tree_root == nullptr) {
				m_tree_root = edge;
				edge->m_tree_parent = nullptr;
			} else {
				SweepEdge *current = m_tree_root;
				for( ; ; ) {
					if(CompareEdgeVertex(current, vertex)) {
						if(current->m_tree_right == nullptr) {
							current->m_tree_right = edge;
							edge->m_tree_parent = current;
							break;
						}
						current = current->m_tree_right;
					} else {
						if(current->m_tree_left == nullptr) {
							current->m_tree_left = edge;
							edge->m_tree_parent = current;
							break;
						}
						current = current->m_tree_left;
					}
				}
			}
		} else {
			if(after->m_tree_right == nullptr) {
				after->m_tree_right = edge;
				edge->m_tree_parent = after;
			} else {
				SweepEdge *current = after->m_tree_right;
				for( ; ; ) {
					if(current->m_tree_left == nullptr) {
						current->m_tree_left = edge;
						edge->m_tree_parent = current;
						break;
					}
					current = current->m_tree_left;
				}
			}
		}
		edge->m_tree_left = nullptr;
		edge->m_tree_right = nullptr;
		edge->m_tree_red = true;

		// rebalance tree
		SweepEdge *current = edge;
		for( ; ; ) {
			if(current->m_tree_parent == nullptr) {
				current->m_tree_red = false;
				break;
			}
			if(!current->m_tree_parent->m_tree_red) {
				break;
			}
			SweepEdge *grandparent = current->m_tree_parent->m_tree_parent;
			assert(grandparent != nullptr);
			if(grandparent->m_tree_left != nullptr && grandparent->m_tree_left->m_tree_red &&
					grandparent->m_tree_right != nullptr && grandparent->m_tree_right->m_tree_red) {
				grandparent->m_tree_left->m_tree_red = false;
				grandparent->m_tree_right->m_tree_red = false;
				assert(!grandparent->m_tree_red);
				grandparent->m_tree_red = true;
				current = grandparent;
				continue;
			}
			if(current->m_tree_parent == grandparent->m_tree_left) {
				if(current->m_tree_parent->m_tree_right == current) {
					//    ,---g---,  =>     ,---g---,
					//  ,-A-,     x  =>   ,-C-,     x
					//  x  ,C,       =>  ,A,  x
					//     b x       =>  x b
					SweepEdge *a = current->m_tree_parent;
					TreeRotateLeft(a);
					current = a;
				}
				//     ,---g---,  =>    ,---a---,
				//   ,-A-,     x  =>  ,-C-,   ,-G-,
				//  ,C,  b        =>  x   x   b   x
				//  x x           =>
				SweepEdge *a = current->m_tree_parent;
				TreeRotateRight(grandparent);
				a->m_tree_red = false;
				grandparent->m_tree_red = true;
			} else {
				if(current->m_tree_parent->m_tree_left == current) {
					//  ,---g---,    =>  ,---g---,
					//  x     ,-A-,  =>  x     ,-C-,
					//       ,C,  x  =>        x  ,A,
					//       x b     =>           b x
					SweepEdge *a = current->m_tree_parent;
					TreeRotateRight(a);
					current = a;
				}
				//  ,---g---,     =>     ,---a---,
				//  x     ,-A-,   =>   ,-G-,   ,-C-,
				//        b  ,C,  =>   x   b   x   x
				//           x x  =>
				SweepEdge *a = current->m_tree_parent;
				TreeRotateLeft(grandparent);
				a->m_tree_red = false;
				grandparent->m_tree_red = true;
			}
			break;
		}

#if POLYMATH_VERIFY
		//TreePrint();
		TreeVerify();
#endif

	}

	void TreeRemove(SweepEdge *edge) {
		assert(edge != nullptr);

		// if the edge has two children, replace it with the leftmost edge in the right subtree
		if(edge->m_tree_left != nullptr && edge->m_tree_right != nullptr) {
			SweepEdge *current = edge->m_tree_right;
			while(current->m_tree_left != nullptr) {
				current = current->m_tree_left;
			}
			TreeSwap(edge, current);
		}

		// check the color of the replacement child
		SweepEdge *child = (edge->m_tree_right == nullptr)? edge->m_tree_left : edge->m_tree_right;
		if(!edge->m_tree_red) {
			if(child != nullptr && child->m_tree_red) {
				child->m_tree_red = false;
			} else {
				SweepEdge *current = edge;
				for( ; ; ) {
					if(current->m_tree_parent == nullptr) {
						break;
					}
					SweepEdge *sibling;
					if(current->m_tree_parent->m_tree_left == current) {
						sibling = current->m_tree_parent->m_tree_right;
						if(sibling->m_tree_red) {
							current->m_tree_parent->m_tree_red = true;
							sibling->m_tree_red = false;
							TreeRotateLeft(current->m_tree_parent);
							sibling = current->m_tree_parent->m_tree_right;
						}
					} else {
						sibling = current->m_tree_parent->m_tree_left;
						if(sibling->m_tree_red) {
							current->m_tree_parent->m_tree_red = true;
							sibling->m_tree_red = false;
							TreeRotateRight(current->m_tree_parent);
							sibling = current->m_tree_parent->m_tree_left;
						}
					}
					assert(sibling != nullptr);
					if(!current->m_tree_parent->m_tree_red && !sibling->m_tree_red &&
							(sibling->m_tree_left == nullptr || !sibling->m_tree_left->m_tree_red) &&
							(sibling->m_tree_right == nullptr || !sibling->m_tree_right->m_tree_red)) {
						sibling->m_tree_red = true;
						current = current->m_tree_parent;
						continue;
					}
					if(current->m_tree_parent->m_tree_red && !sibling->m_tree_red &&
							(sibling->m_tree_left == nullptr || !sibling->m_tree_left->m_tree_red) &&
							(sibling->m_tree_right == nullptr || !sibling->m_tree_right->m_tree_red)) {
						sibling->m_tree_red = true;
						current->m_tree_parent->m_tree_red = false;
						break;
					}
					if(!sibling->m_tree_red) {
						if(current->m_tree_parent->m_tree_left == current) {
							if((sibling->m_tree_right == nullptr || !sibling->m_tree_right->m_tree_red) &&
									(sibling->m_tree_left != nullptr && sibling->m_tree_left->m_tree_red)) {
								sibling->m_tree_red = true;
								sibling->m_tree_left->m_tree_red = false;
								sibling = TreeRotateRight(sibling);
							}
						} else {
							if((sibling->m_tree_left == nullptr || !sibling->m_tree_left->m_tree_red) &&
									(sibling->m_tree_right != nullptr && sibling->m_tree_right->m_tree_red)) {
								sibling->m_tree_red = true;
								sibling->m_tree_right->m_tree_red = false;
								sibling = TreeRotateLeft(sibling);
							}
						}
					}
					sibling->m_tree_red = current->m_tree_parent->m_tree_red;
					current->m_tree_parent->m_tree_red = false;
					if(current->m_tree_parent->m_tree_left == current) {
						assert(sibling->m_tree_right->m_tree_red);
						sibling->m_tree_right->m_tree_red = false;
						TreeRotateLeft(current->m_tree_parent);
					} else {
						assert(sibling->m_tree_left->m_tree_red);
						sibling->m_tree_left->m_tree_red = false;
						TreeRotateRight(current->m_tree_parent);
					}
					break;
				}
			}
		}

		// replace edge with child
		if(child != nullptr)
			child->m_tree_parent = edge->m_tree_parent;
		if(edge->m_tree_parent == nullptr) {
			m_tree_root = child;
		} else if(edge->m_tree_parent->m_tree_left == edge) {
			edge->m_tree_parent->m_tree_left = child;
		} else {
			edge->m_tree_parent->m_tree_right = child;
		}

#if POLYMATH_VERIFY
		//TreePrint();
		TreeVerify();
#endif

	}

	SweepEdge* TreeFirst() {
		if(m_tree_root == nullptr)
			return nullptr;
		SweepEdge *current = m_tree_root;
		while(current->m_tree_left != nullptr) {
			current = current->m_tree_left;
		}
		return current;
	}

	SweepEdge* TreeLast() {
		if(m_tree_root == nullptr)
			return nullptr;
		SweepEdge *current = m_tree_root;
		while(current->m_tree_right != nullptr) {
			current = current->m_tree_right;
		}
		return current;
	}

	static SweepEdge* TreeNext(SweepEdge *edge) {
		assert(edge != nullptr);
		if(edge->m_tree_right != nullptr) {
			SweepEdge *current = edge->m_tree_right;
			while(current->m_tree_left != nullptr) {
				current = current->m_tree_left;
			}
			return current;
		}
		SweepEdge *current = edge;
		while(current->m_tree_parent != nullptr) {
			if(current->m_tree_parent->m_tree_left == current)
				return current->m_tree_parent;
			current = current->m_tree_parent;
		}
		return nullptr;
	}

	static SweepEdge* TreePrevious(SweepEdge *edge) {
		assert(edge != nullptr);
		if(edge->m_tree_left != nullptr) {
			SweepEdge *current = edge->m_tree_left;
			while(current->m_tree_right != nullptr) {
				current = current->m_tree_right;
			}
			return current;
		}
		SweepEdge *current = edge;
		while(current->m_tree_parent != nullptr) {
			if(current->m_tree_parent->m_tree_right == current)
				return current->m_tree_parent;
			current = current->m_tree_parent;
		}
		return nullptr;
	}

#if POLYMATH_VERIFY

	void HeapPrint() {
		std::cout << "Heap:" << std::endl;
		for(size_t i = 0; i < m_heap.size(); ++i) {
			if(i != 0 && (i & (i + 1)) == 0)
				std::cout << std::endl;
			std::cout << m_heap[i]->m_heap_vertex.y << "   ";
		}
		std::cout << std::endl;
	}

	void HeapVerify() {

		for(size_t i = 0; i < m_heap.size(); ++i) {

			// verify the index
			assert(m_heap[i]->m_heap_index == i);

			// verify that the node has a lower y value than both childs
			size_t child1 = i * 2 + 1, child2 = child1 + 1;
			if(child1 < m_heap.size())
				assert(m_heap[i]->m_heap_vertex.x <= m_heap[child1]->m_heap_vertex.x);
			if(child2 < m_heap.size())
				assert(m_heap[i]->m_heap_vertex.x <= m_heap[child2]->m_heap_vertex.x);

		}

	}

#endif

	void HeapInsert(SweepEdge *edge) {
		assert(edge->m_heap_index == INDEX_NONE);

		// add to end of heap
		size_t current = m_heap.size();
		m_heap.push_back(edge);
		edge->m_heap_index = current;

		// sift up
		while(current != 0) {
			size_t parent = (current - 1) / 2;
			if(m_heap[parent]->m_heap_vertex.x <= m_heap[current]->m_heap_vertex.x)
				break;
			std::swap(m_heap[parent], m_heap[current]);
			m_heap[parent]->m_heap_index = parent;
			m_heap[current]->m_heap_index = current;
			current = parent;
		}

#if POLYMATH_VERIFY
		//HeapPrint();
		HeapVerify();
#endif

	}

	void HeapRemove(SweepEdge *edge) {
		assert(edge->m_heap_index != INDEX_NONE);

		// replace with end of heap and remove end
		size_t current = edge->m_heap_index;
		m_heap[current] = m_heap.back();
		m_heap[current]->m_heap_index = current;
		m_heap.pop_back();
		edge->m_heap_index = INDEX_NONE;

		// sift up
		size_t start = current;
		while(current != 0) {
			size_t parent = (current - 1) / 2;
			if(m_heap[parent]->m_heap_vertex.x <= m_heap[current]->m_heap_vertex.x)
				break;
			std::swap(m_heap[parent], m_heap[current]);
			m_heap[parent]->m_heap_index = parent;
			m_heap[current]->m_heap_index = current;
			current = parent;
		}

		// sift down
		if(current == start) {
			while(current * 2 + 1 < m_heap.size()) {
				size_t child1 = current * 2 + 1, child2 = child1 + 1;
				if(child2 < m_heap.size() && m_heap[child2]->m_heap_vertex.x < m_heap[child1]->m_heap_vertex.x) {
					if(m_heap[current]->m_heap_vertex.x <= m_heap[child2]->m_heap_vertex.x)
						break;
					std::swap(m_heap[current], m_heap[child2]);
					m_heap[current]->m_heap_index = current;
					m_heap[child2]->m_heap_index = child2;
					current = child2;
				} else {
					if(m_heap[current]->m_heap_vertex.x <= m_heap[child1]->m_heap_vertex.x)
						break;
					std::swap(m_heap[current], m_heap[child1]);
					m_heap[current]->m_heap_index = current;
					m_heap[child1]->m_heap_index = child1;
					current = child1;
				}
			}
		}

#if POLYMATH_VERIFY
		//HeapPrint();
		HeapVerify();
#endif

	}

	SweepEdge* HeapTop() {
		if(m_heap.empty())
			return nullptr;
		return m_heap.front();
	}

#if POLYMATH_VERIFY

	void WindingNumberPrint() {
		std::cout << "<" << 0 << ">" << std::endl;
		for(SweepEdge *edge = TreeFirst(); edge != nullptr; edge = TreeNext(edge)) {
			std::cout << ((edge->m_output_vertex == nullptr)? " " : "-") << " [" << edge->m_winding_weight << "] " << edge->m_vertex_first << " " << edge->m_vertex_last << std::endl;
			std::cout << "<" << edge->m_winding_number << ">" << std::endl;
		}
		std::cout << std::endl;
	}

	void WindingNumberVerify() {
		int64_t winding_number = 0;
		bool w1 = WindingEngine::WindingRule(winding_number);
		for(SweepEdge *edge = TreeFirst(); edge != nullptr; edge = TreeNext(edge)) {
			winding_number += edge->m_winding_weight;
			assert(edge->m_winding_number == winding_number);
			bool w2 = WindingEngine::WindingRule(winding_number);
			if(w1 != w2) {
				assert(edge->m_output_vertex != nullptr);
				assert(edge->m_output_vertex_forward == w1);
			}
			w1 = w2;
		}
	}

#endif

	OutputVertex* AddOutputVertex(Vertex vertex) {
		if(m_output_vertex_batch_used == OUTPUT_VERTEX_BATCH_SIZE) {
			std::unique_ptr<OutputVertex[]> mem(new OutputVertex[OUTPUT_VERTEX_BATCH_SIZE]);
			m_output_vertex_batches.push_back(std::move(mem));
			m_output_vertex_batch_used = 0;
		}
		OutputVertex *batch = m_output_vertex_batches.back().get();
		OutputVertex *v = &batch[m_output_vertex_batch_used];
		v->m_vertex = vertex;
		v->m_next = nullptr; // needed for visualization
		++m_output_vertex_batch_used;
		return v;
	}

	void UpdateIntersection(SweepEdge *edge1, SweepEdge *edge2){
		assert(edge1 == nullptr || edge1 != edge2);

		if(edge1 == nullptr)
			return;
		if(edge1->m_heap_index != INDEX_NONE)
			HeapRemove(edge1);

		if(edge2 == nullptr)
			return;
		if(IntersectEdgeEdge(edge1, edge2, edge1->m_heap_vertex)) {
			HeapInsert(edge1);
		}

	}

	void RemoveIntersection(SweepEdge *edge) {
		assert(edge != nullptr);
		if(edge->m_heap_index != INDEX_NONE) {
			HeapRemove(edge);
		}
	}

	void ProcessIntersection(SweepEdge *edge, Vertex intersection_vertex) {

		// get surrounding edges
		SweepEdge *edge1 = edge, *edge2 = TreeNext(edge);
		SweepEdge *edge_prev = TreePrevious(edge1), *edge_next = TreeNext(edge2);

		// swap edges
		TreeSwap(edge1, edge2);
		std::swap(edge1, edge2);

		// update intersections
		RemoveIntersection(edge1);
		UpdateIntersection(edge_prev, edge1);
		UpdateIntersection(edge2, edge_next);

		// update winding numbers
		edge2->m_winding_number = edge1->m_winding_number;
		edge1->m_winding_number -= edge2->m_winding_weight;

		// if both edges have output vertices, combine them
		if(edge1->m_output_vertex != nullptr && edge2->m_output_vertex != nullptr) {
			assert(edge1->m_output_vertex_forward != edge2->m_output_vertex_forward);
			if(edge1->m_output_vertex_forward) {
				OutputVertex *output_vertex = AddOutputVertex(intersection_vertex);
				edge1->m_output_vertex->m_next = output_vertex;
				output_vertex->m_next = edge2->m_output_vertex;
			} else {
				OutputVertex *output_vertex = AddOutputVertex(intersection_vertex);
				edge2->m_output_vertex->m_next = output_vertex;
				output_vertex->m_next = edge1->m_output_vertex;
			}
			edge1->m_output_vertex = nullptr;
			edge2->m_output_vertex = nullptr;
		}

		if(edge1->m_output_vertex == nullptr && edge2->m_output_vertex == nullptr) {

			// if there are no output vertices, create them if necessary
			bool w1 = WindingEngine::WindingRule(edge1->m_winding_number), w2 = WindingEngine::WindingRule(edge2->m_winding_number);
			if(w1 != w2) {
				OutputVertex *output_vertex = AddOutputVertex(intersection_vertex);
				edge1->m_output_vertex = output_vertex;
				edge1->m_output_vertex_forward = w2;
				edge2->m_output_vertex = output_vertex;
				edge2->m_output_vertex_forward = w1;
			}

		} else {

			// if only one edge has an output vertex, propagate it
			SweepEdge *edge_old = (edge1->m_output_vertex == nullptr)? edge2 : edge1;
			SweepEdge *edge_new = (WindingEngine::WindingRule(edge1->m_winding_number) == edge_old->m_output_vertex_forward)? edge2 : edge1;
			if(edge_old != edge_new) {
				if(edge_old->m_output_vertex_forward) {
					OutputVertex *output_vertex = AddOutputVertex(intersection_vertex);
					edge_old->m_output_vertex->m_next = output_vertex;
					edge_old->m_output_vertex = nullptr;
					edge_new->m_output_vertex = output_vertex;
				} else {
					OutputVertex *output_vertex = AddOutputVertex(intersection_vertex);
					output_vertex->m_next = edge_old->m_output_vertex;
					edge_old->m_output_vertex = nullptr;
					edge_new->m_output_vertex = output_vertex;
				}
				edge_new->m_output_vertex_forward = edge_old->m_output_vertex_forward;
			}

		}

#if POLYMATH_VERIFY
		//WindingNumberPrint();
		WindingNumberVerify();
#endif

	}

	void ProcessStartVertex(SweepVertex *vertex) {

		// add sweep edges
		SweepEdge *edge1, *edge2;
		edge1 = AddSweepEdge();
		edge1->m_vertex_first = vertex->m_vertex;
		edge1->m_vertex_last = vertex->m_loop_prev->m_vertex;
		edge1->m_winding_weight = -vertex->m_winding_weight;
		edge2 = AddSweepEdge();
		edge2->m_vertex_first = vertex->m_vertex;
		edge2->m_vertex_last = vertex->m_loop_next->m_vertex;
		edge2->m_winding_weight = vertex->m_winding_weight;

		// set vertex pointers
		vertex->m_loop_prev->m_sweep_edge = edge1;
		vertex->m_sweep_edge = edge2;

		// check the order of the edges
		SingleType a_x = vertex->m_vertex.x;
		SingleType a_y = vertex->m_vertex.y;
		SingleType b_x = vertex->m_loop_prev->m_vertex.x;
		SingleType b_y = vertex->m_loop_prev->m_vertex.y;
		SingleType c_x = vertex->m_loop_next->m_vertex.x;
		SingleType c_y = vertex->m_loop_next->m_vertex.y;
		if(!NumericalEngine::OrientationTest(a_x, a_y, b_x, b_y, c_x, c_y, true)) {
			std::swap(edge1, edge2);
		}

		// insert edges into tree
		TreeInsert(edge1, vertex);
		TreeInsert(edge2, vertex, edge1);

		// update intersections
		SweepEdge *edge0 = TreePrevious(edge1), *edge3 = TreeNext(edge2);
		UpdateIntersection(edge0, edge1);
		UpdateIntersection(edge2, edge3);

		// update winding numbers
		int64_t winding_number = (edge0 == nullptr)? 0 : edge0->m_winding_number;
		edge1->m_winding_number = winding_number + edge1->m_winding_weight;
		edge2->m_winding_number = winding_number;

		// add output vertex
		bool w1 = WindingEngine::WindingRule(edge1->m_winding_number), w2 = WindingEngine::WindingRule(edge2->m_winding_number);
		if(w1 == w2) {
			edge1->m_output_vertex = nullptr;
			edge2->m_output_vertex = nullptr;
		} else {
			OutputVertex *output_vertex = AddOutputVertex(vertex->m_vertex);
			edge1->m_output_vertex = output_vertex;
			edge1->m_output_vertex_forward = w2;
			edge2->m_output_vertex = output_vertex;
			edge2->m_output_vertex_forward = w1;
		}

#if POLYMATH_VERIFY
		//WindingNumberPrint();
		WindingNumberVerify();
#endif

	}

	void ProcessMiddleVertex(SweepVertex *vertex) {

		// update edge pointer
		SweepVertex *vertex_next;
		SweepEdge *edge;
		if(vertex->m_edge_forward) {
			vertex_next = vertex->m_loop_next;
			edge = vertex->m_loop_prev->m_sweep_edge;
			vertex->m_sweep_edge = edge;
		} else {
			vertex_next = vertex->m_loop_prev;
			edge = vertex->m_sweep_edge;
			vertex->m_loop_prev->m_sweep_edge = edge;
		}

		// update vertex pointers
		edge->m_vertex_first = vertex->m_vertex;
		edge->m_vertex_last = vertex_next->m_vertex;

		// update intersections
		SweepEdge *edge_prev = TreePrevious(edge), *edge_next = TreeNext(edge);
		UpdateIntersection(edge_prev, edge);
		UpdateIntersection(edge, edge_next);

		// update output vertex
		if(edge->m_output_vertex != nullptr) {
			if(edge->m_output_vertex_forward) {
				OutputVertex *output_vertex = AddOutputVertex(vertex->m_vertex);
				edge->m_output_vertex->m_next = output_vertex;
				edge->m_output_vertex = output_vertex;
			} else {
				OutputVertex *output_vertex = AddOutputVertex(vertex->m_vertex);
				output_vertex->m_next = edge->m_output_vertex;
				edge->m_output_vertex = output_vertex;
			}
		}

#if POLYMATH_VERIFY
		//WindingNumberPrint();
		WindingNumberVerify();
#endif

	}

	void ProcessStopVertex(SweepVertex *vertex) {

		// The two edges are supposed to be neighbours in the tree, but due to rounding errors it is possible that there are other edges in between.
		// Also, we don't know which edge is on the left side. So we first need to search in both directions until we find the other edge.
		SweepEdge *edge1 = vertex->m_loop_prev->m_sweep_edge, *edge2 = vertex->m_loop_prev->m_sweep_edge;
		for( ; ; ) {

			// one step left
			edge1 = TreePrevious(edge1);
			if(edge1 == nullptr) {
				do {
					edge2 = TreeNext(edge2);
					assert(edge2 != nullptr);
				} while(edge2 != vertex->m_sweep_edge);
				edge1 = vertex->m_loop_prev->m_sweep_edge;
				break;
			}
			if(edge1 == vertex->m_sweep_edge) {
				edge2 = vertex->m_loop_prev->m_sweep_edge;
				break;
			}

			// one step right
			edge2 = TreeNext(edge2);
			if(edge2 == nullptr) {
				do {
					edge1 = TreePrevious(edge1);
					assert(edge1 != nullptr);
				} while(edge1 != vertex->m_sweep_edge);
				edge2 = vertex->m_loop_prev->m_sweep_edge;
				break;
			}
			if(edge2 == vertex->m_sweep_edge) {
				edge1 = vertex->m_loop_prev->m_sweep_edge;
				break;
			}

		}

		// force intersections for edges that are stuck between the two edges that are going to merge
		SweepEdge *next = TreeNext(edge1);
		while(next != edge2) {
			ProcessIntersection(edge1, vertex->m_vertex);
			next = TreeNext(edge1);
		}

		// remove edges from tree
		SweepEdge *edge_prev = TreePrevious(edge1), *edge_next = TreeNext(edge2);
		TreeRemove(edge1);
		TreeRemove(edge2);

		// update intersections
		assert(edge1->m_heap_index == INDEX_NONE); // edge1 can't intersect edge2 because they are connected
		RemoveIntersection(edge2); // theoretically there shouldn't be an intersection, but this is necessary because of rounding errors
		UpdateIntersection(edge_prev, edge_next);

		// update output vertices
		if(edge1->m_output_vertex != nullptr) {
			assert(edge2->m_output_vertex != nullptr);
			assert(edge1->m_output_vertex_forward != edge2->m_output_vertex_forward);
			if(edge1->m_output_vertex_forward) {
				OutputVertex *output_vertex = AddOutputVertex(vertex->m_vertex);
				edge1->m_output_vertex->m_next = output_vertex;
				output_vertex->m_next = edge2->m_output_vertex;
			} else {
				OutputVertex *output_vertex = AddOutputVertex(vertex->m_vertex);
				edge2->m_output_vertex->m_next = output_vertex;
				output_vertex->m_next = edge1->m_output_vertex;
			}
		} else {
			assert(edge2->m_output_vertex == nullptr);
		}

		// remove sweep edges
		RemoveSweepEdge(edge1);
		RemoveSweepEdge(edge2);

#if POLYMATH_VERIFY
		//WindingNumberPrint();
		WindingNumberVerify();
#endif

	}

public:

	SweepEngine(const Polygon<Vertex> &polygon) {

		// initialize
		m_current_vertex = 0;
		m_sweep_edge_free_list = nullptr;
		m_tree_root = nullptr;
		m_output_vertex_batch_used = OUTPUT_VERTEX_BATCH_SIZE;

		// count the total number of vertices
		size_t total_vertices = 0;
		size_t index = 0;
		for(size_t loop = 0; loop < polygon.GetLoopCount(); ++loop) {
			size_t end = polygon.GetLoopEnd(loop);
			if(end - index >= 3) {
				total_vertices += end - index;
			}
			index = end;
		}

		// import the polygon
		m_vertex_pool.resize(total_vertices);
		m_vertex_queue.resize(total_vertices);
		index = 0;
		size_t current = 0;
		for(size_t loop = 0; loop < polygon.GetLoopCount(); ++loop) {

			// get loop
			size_t end = polygon.GetLoopEnd(loop);
			int64_t winding_weight = polygon.GetLoopWindingWeight(loop);

			// ignore polygons with less than three vertices
			if(end - index < 3) {
				index = end;
				continue;
			}

			// handle all vertices
			SweepVertex *first = nullptr, *last = nullptr;
			for( ; index < end; ++index) {

				// get a new vertex and add it to the queue
				SweepVertex *v = &m_vertex_pool[current];
				m_vertex_queue[current] = v;
				++current;

				// copy vertex properties
				v->m_vertex = polygon.GetVertex(index);
				v->m_winding_weight = winding_weight;

				// add to the loop
				if(first == nullptr) {
					first = v;
					last = v;
				} else {
					last->m_loop_next = v;
					last->m_edge_forward = CompareVertexVertex(last, v);
					v->m_loop_prev = last;
					last = v;
				}

			}

			// complete the loop
			last->m_loop_next = first;
			last->m_edge_forward = CompareVertexVertex(last, first);
			first->m_loop_prev = last;

		}
		assert(current == total_vertices);

		// sort the vertices from top to bottom
		std::sort(m_vertex_queue.begin(), m_vertex_queue.end(), CompareVertexVertex);

	}

	template<typename VisualizationCallback = void()>
	void Process(VisualizationCallback &&visualization_callback = DummyVisualizationCallback) {

		// iterate through sorted vertices
		for(m_current_vertex = 0; m_current_vertex < m_vertex_queue.size(); ++m_current_vertex) {
			SweepVertex *v = m_vertex_queue[m_current_vertex];

			// process required intersections
			for( ; ; ) {
				SweepEdge *w = HeapTop();
				if(w == nullptr || w->m_heap_vertex.x > NumericalEngine::SingleToDouble(v->m_vertex.x))
					break;
				visualization_callback();
				ProcessIntersection(w, Vertex(NumericalEngine::DoubleToSingle(w->m_heap_vertex.x), NumericalEngine::DoubleToSingle(w->m_heap_vertex.y)));
			}

			// process the new vertex
			visualization_callback();
			if(v->m_loop_prev->m_edge_forward == v->m_edge_forward) {
				ProcessMiddleVertex(v);
			} else if(v->m_edge_forward) {
				ProcessStartVertex(v);
			} else {
				ProcessStopVertex(v);
			}

		}

		assert(TreeFirst() == nullptr);
		assert(HeapTop() == nullptr);

	}

	Visualization<Vertex> Visualize() {
		Visualization<Vertex> result;

		// sweepline
		result.m_has_current_vertex = (m_current_vertex < m_vertex_queue.size());
		if(result.m_has_current_vertex) {
			SweepVertex *v = m_vertex_queue[m_current_vertex];
			SweepEdge *w = HeapTop();
			if(w == nullptr || w->m_heap_vertex.x > NumericalEngine::SingleToDouble(v->m_vertex.x)) {
				result.m_current_vertex = v->m_vertex;
			} else {
				result.m_current_vertex = Vertex(NumericalEngine::DoubleToSingle(w->m_heap_vertex.x), NumericalEngine::DoubleToSingle(w->m_heap_vertex.y));
			}
		}

		// tree
		for(SweepEdge *w = TreeFirst(); w != nullptr; w = TreeNext(w)) {
			result.m_sweep_edges.emplace_back();
			auto &edge = result.m_sweep_edges.back();
			edge.m_edge_vertices[0] = w->m_vertex_first;
			edge.m_edge_vertices[1] = w->m_vertex_last;
			edge.m_has_intersection = (w->m_heap_index != INDEX_NONE);
			edge.m_intersection_vertex = Vertex(NumericalEngine::DoubleToSingle(w->m_heap_vertex.x), NumericalEngine::DoubleToSingle(w->m_heap_vertex.y));
			edge.m_winding_number = w->m_winding_number;
		}

		// output polygon
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *pool = m_output_vertex_batches[i].get();
			size_t pool_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < pool_size; ++j) {
				OutputVertex *v = pool + j;
				if(v->m_next != nullptr) {
					result.m_output_edges.emplace_back();
					auto &edge = result.m_output_edges.back();
					edge.m_edge_vertices[0] = v->m_vertex;
					edge.m_edge_vertices[1] = v->m_next->m_vertex;
				}
			}
		}

		return result;
	}

	Polygon<Vertex> Result() {
		Polygon<Vertex> result;

		// reserve space for all output vertices
		result.Clear();
		result.ReserveVertices(m_output_vertex_batches.size() * OUTPUT_VERTEX_BATCH_SIZE + m_output_vertex_batch_used - OUTPUT_VERTEX_BATCH_SIZE);

		// fill polygon with output vertex data
		for(size_t i = 0; i < m_output_vertex_batches.size(); ++i) {
			OutputVertex *batch = m_output_vertex_batches[i].get();
			size_t batch_size = (i == m_output_vertex_batches.size() - 1)? m_output_vertex_batch_used : OUTPUT_VERTEX_BATCH_SIZE;
			for(size_t j = 0; j < batch_size; ++j) {
				OutputVertex *v = batch + j;
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

}
