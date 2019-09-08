#pragma once

#include "Common.h"

namespace PolyMath {

template<class SweepEdge>
class SweepTree_Basic {

public:
	struct Node {
		SweepEdge *m_tree_parent, *m_tree_left, *m_tree_right;
		bool m_tree_red;
	};

private:
	SweepEdge *m_tree_root;

private:
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

	void RebalanceInsert(SweepEdge *node) {
		assert(node != nullptr);
		if(node->m_tree_parent == nullptr) {
			node->m_tree_red = false;
			return;
		}
		if(!node->m_tree_parent->m_tree_red) {
			return;
		}
		SweepEdge *grandparent = node->m_tree_parent->m_tree_parent;
		assert(grandparent != nullptr);
		if(grandparent->m_tree_left != nullptr && grandparent->m_tree_left->m_tree_red &&
				grandparent->m_tree_right != nullptr && grandparent->m_tree_right->m_tree_red) {
			grandparent->m_tree_left->m_tree_red = false;
			grandparent->m_tree_right->m_tree_red = false;
			assert(!grandparent->m_tree_red);
			grandparent->m_tree_red = true;
			RebalanceInsert(grandparent);
		} else if(node->m_tree_parent == grandparent->m_tree_left) {
			if(node->m_tree_parent->m_tree_right == node) {
				//    ,---g---,  =>     ,---g---,
				//  ,-A-,     x  =>   ,-C-,     x
				//  x  ,C,       =>  ,A,  x
				//     b x       =>  x b
				SweepEdge *a = node->m_tree_parent;
				TreeRotateLeft(a);
				node = a;
			}
			//     ,---g---,  =>    ,---a---,
			//   ,-A-,     x  =>  ,-C-,   ,-G-,
			//  ,C,  b        =>  x   x   b   x
			//  x x           =>
			SweepEdge *a = node->m_tree_parent;
			TreeRotateRight(grandparent);
			a->m_tree_red = false;
			grandparent->m_tree_red = true;
		} else {
			if(node->m_tree_parent->m_tree_left == node) {
				//  ,---g---,    =>  ,---g---,
				//  x     ,-A-,  =>  x     ,-C-,
				//       ,C,  x  =>        x  ,A,
				//       x b     =>           b x
				SweepEdge *a = node->m_tree_parent;
				TreeRotateRight(a);
				node = a;
			}
			//  ,---g---,     =>     ,---a---,
			//  x     ,-A-,   =>   ,-G-,   ,-C-,
			//        b  ,C,  =>   x   b   x   x
			//           x x  =>
			SweepEdge *a = node->m_tree_parent;
			TreeRotateLeft(grandparent);
			a->m_tree_red = false;
			grandparent->m_tree_red = true;
		}
	}

public:
	SweepTree_Basic() {
		m_tree_root = nullptr;
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

	static SweepEdge* TreeNext(SweepEdge *node) {
		assert(node != nullptr);
		if(node->m_tree_right != nullptr) {
			SweepEdge *current = node->m_tree_right;
			while(current->m_tree_left != nullptr) {
				current = current->m_tree_left;
			}
			return current;
		}
		SweepEdge *current = node;
		while(current->m_tree_parent != nullptr) {
			if(current->m_tree_parent->m_tree_left == current)
				return current->m_tree_parent;
			current = current->m_tree_parent;
		}
		return nullptr;
	}

	static SweepEdge* TreePrevious(SweepEdge *node) {
		assert(node != nullptr);
		if(node->m_tree_left != nullptr) {
			SweepEdge *current = node->m_tree_left;
			while(current->m_tree_right != nullptr) {
				current = current->m_tree_right;
			}
			return current;
		}
		SweepEdge *current = node;
		while(current->m_tree_parent != nullptr) {
			if(current->m_tree_parent->m_tree_right == current)
				return current->m_tree_parent;
			current = current->m_tree_parent;
		}
		return nullptr;
	}

	void TreeSwap(SweepEdge *node1, SweepEdge *node2) {
		assert(node1 != nullptr);
		assert(node2 != nullptr);

		// swapping two edges is harder than it sounds because we need to handle all possible edge cases
		// copy original pointers to avoid confusion
		SweepEdge *p1 = node1->m_tree_parent, *p2 = node2->m_tree_parent;
		SweepEdge *l1 = node1->m_tree_left, *l2 = node2->m_tree_left;
		SweepEdge *r1 = node1->m_tree_right, *r2 = node2->m_tree_right;

		// swap colors
		std::swap(node1->m_tree_red, node2->m_tree_red);

		// fix internal pointers
		node1->m_tree_parent = (p2 == node1)? node2 : p2;
		node1->m_tree_left = (l2 == node1)? node2 : l2;
		node1->m_tree_right = (r2 == node1)? node2 : r2;
		node2->m_tree_parent = (p1 == node2)? node1 : p1;
		node2->m_tree_left = (l1 == node2)? node1 : l1;
		node2->m_tree_right = (r1 == node2)? node1 : r1;

		// fix pointers from parents
		if(p1 != node2) {
			if(p1 == nullptr) {
				m_tree_root = node2;
			} else if(p1->m_tree_left == node1) {
				p1->m_tree_left = node2;
			} else {
				p1->m_tree_right = node2;
			}
		}
		if(p2 != node1) {
			if(p2 == nullptr) {
				m_tree_root = node1;
			} else if(p2->m_tree_left == node2) {
				p2->m_tree_left = node1;
			} else {
				p2->m_tree_right = node1;
			}
		}

		// fix pointers from children
		if(l1 != nullptr && l1 != node2)
			l1->m_tree_parent = node2;
		if(r1 != nullptr && r1 != node2)
			r1->m_tree_parent = node2;
		if(l2 != nullptr && l2 != node1)
			l2->m_tree_parent = node1;
		if(r2 != nullptr && r2 != node1)
			r2->m_tree_parent = node1;

	}

	template<typename Compare>
	void TreeInsertAt(SweepEdge *node, Compare &&comp) {
		assert(node != nullptr);

		// insert edge
		if(m_tree_root == nullptr) {
			m_tree_root = node;
			node->m_tree_parent = nullptr;
		} else {
			SweepEdge *current = m_tree_root;
			for( ; ; ) {
				if(comp(current)) {
					if(current->m_tree_right == nullptr) {
						current->m_tree_right = node;
						node->m_tree_parent = current;
						break;
					}
					current = current->m_tree_right;
				} else {
					if(current->m_tree_left == nullptr) {
						current->m_tree_left = node;
						node->m_tree_parent = current;
						break;
					}
					current = current->m_tree_left;
				}
			}
		}
		node->m_tree_left = nullptr;
		node->m_tree_right = nullptr;
		node->m_tree_red = true;

		// rebalance
		RebalanceInsert(node);

	}

	void TreeInsertAfter(SweepEdge *node, SweepEdge *after) {
		assert(node != nullptr);
		assert(after != nullptr);

		// insert edge
		if(after->m_tree_right == nullptr) {
			after->m_tree_right = node;
			node->m_tree_parent = after;
		} else {
			SweepEdge *current = after->m_tree_right;
			for( ; ; ) {
				if(current->m_tree_left == nullptr) {
					current->m_tree_left = node;
					node->m_tree_parent = current;
					break;
				}
				current = current->m_tree_left;
			}
		}
		node->m_tree_left = nullptr;
		node->m_tree_right = nullptr;
		node->m_tree_red = true;

		// rebalance
		RebalanceInsert(node);

	}

	void TreeRemove(SweepEdge *node) {
		assert(node != nullptr);

		// if the edge has two children, replace it with the leftmost edge in the right subtree
		if(node->m_tree_left != nullptr && node->m_tree_right != nullptr) {
			SweepEdge *current = node->m_tree_right;
			while(current->m_tree_left != nullptr) {
				current = current->m_tree_left;
			}
			TreeSwap(node, current);
		}

		// check the color of the replacement child
		SweepEdge *child = (node->m_tree_right == nullptr)? node->m_tree_left : node->m_tree_right;
		if(!node->m_tree_red) {
			if(child != nullptr && child->m_tree_red) {
				child->m_tree_red = false;
			} else {
				SweepEdge *current = node;
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
			child->m_tree_parent = node->m_tree_parent;
		if(node->m_tree_parent == nullptr) {
			m_tree_root = child;
		} else if(node->m_tree_parent->m_tree_left == node) {
			node->m_tree_parent->m_tree_left = child;
		} else {
			node->m_tree_parent->m_tree_right = child;
		}

	}

};


template<class SweepEdge>
class SweepTree_Basic2 {

public:
	struct Node {
		SweepEdge *m_tree_parent, *m_tree_left, *m_tree_right, *m_tree_prev, *m_tree_next;
		bool m_tree_red;
	};

private:
	SweepEdge *m_tree_root;

private:
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

	void RebalanceInsert(SweepEdge *node) {
		assert(node != nullptr);
		if(node->m_tree_parent == nullptr) {
			node->m_tree_red = false;
			return;
		}
		if(!node->m_tree_parent->m_tree_red) {
			return;
		}
		SweepEdge *grandparent = node->m_tree_parent->m_tree_parent;
		assert(grandparent != nullptr);
		if(grandparent->m_tree_left != nullptr && grandparent->m_tree_left->m_tree_red &&
				grandparent->m_tree_right != nullptr && grandparent->m_tree_right->m_tree_red) {
			grandparent->m_tree_left->m_tree_red = false;
			grandparent->m_tree_right->m_tree_red = false;
			assert(!grandparent->m_tree_red);
			grandparent->m_tree_red = true;
			RebalanceInsert(grandparent);
		} else if(node->m_tree_parent == grandparent->m_tree_left) {
			if(node->m_tree_parent->m_tree_right == node) {
				//    ,---g---,  =>     ,---g---,
				//  ,-A-,     x  =>   ,-C-,     x
				//  x  ,C,       =>  ,A,  x
				//     b x       =>  x b
				SweepEdge *a = node->m_tree_parent;
				TreeRotateLeft(a);
				node = a;
			}
			//     ,---g---,  =>    ,---a---,
			//   ,-A-,     x  =>  ,-C-,   ,-G-,
			//  ,C,  b        =>  x   x   b   x
			//  x x           =>
			SweepEdge *a = node->m_tree_parent;
			TreeRotateRight(grandparent);
			a->m_tree_red = false;
			grandparent->m_tree_red = true;
		} else {
			if(node->m_tree_parent->m_tree_left == node) {
				//  ,---g---,    =>  ,---g---,
				//  x     ,-A-,  =>  x     ,-C-,
				//       ,C,  x  =>        x  ,A,
				//       x b     =>           b x
				SweepEdge *a = node->m_tree_parent;
				TreeRotateRight(a);
				node = a;
			}
			//  ,---g---,     =>     ,---a---,
			//  x     ,-A-,   =>   ,-G-,   ,-C-,
			//        b  ,C,  =>   x   b   x   x
			//           x x  =>
			SweepEdge *a = node->m_tree_parent;
			TreeRotateLeft(grandparent);
			a->m_tree_red = false;
			grandparent->m_tree_red = true;
		}
	}

public:
	SweepTree_Basic2() {
		m_tree_root = nullptr;
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

	static SweepEdge* TreeNext(SweepEdge *node) {
		assert(node != nullptr);
		return node->m_tree_next;
	}

	static SweepEdge* TreePrevious(SweepEdge *node) {
		assert(node != nullptr);
		return node->m_tree_prev;
	}

	void TreeSwap(SweepEdge *node1, SweepEdge *node2) {
		assert(node1 != nullptr);
		assert(node2 != nullptr);

		// swapping two edges is harder than it sounds because we need to handle all possible edge cases
		// copy original pointers to avoid confusion
		SweepEdge *p1 = node1->m_tree_parent, *p2 = node2->m_tree_parent;
		SweepEdge *l1 = node1->m_tree_left, *l2 = node2->m_tree_left;
		SweepEdge *r1 = node1->m_tree_right, *r2 = node2->m_tree_right;

		// swap colors
		std::swap(node1->m_tree_red, node2->m_tree_red);

		// fix internal pointers
		node1->m_tree_parent = (p2 == node1)? node2 : p2;
		node1->m_tree_left = (l2 == node1)? node2 : l2;
		node1->m_tree_right = (r2 == node1)? node2 : r2;
		node2->m_tree_parent = (p1 == node2)? node1 : p1;
		node2->m_tree_left = (l1 == node2)? node1 : l1;
		node2->m_tree_right = (r1 == node2)? node1 : r1;

		// fix pointers from parents
		if(p1 != node2) {
			if(p1 == nullptr) {
				m_tree_root = node2;
			} else if(p1->m_tree_left == node1) {
				p1->m_tree_left = node2;
			} else {
				p1->m_tree_right = node2;
			}
		}
		if(p2 != node1) {
			if(p2 == nullptr) {
				m_tree_root = node1;
			} else if(p2->m_tree_left == node2) {
				p2->m_tree_left = node1;
			} else {
				p2->m_tree_right = node1;
			}
		}

		// fix pointers from children
		if(l1 != nullptr && l1 != node2)
			l1->m_tree_parent = node2;
		if(r1 != nullptr && r1 != node2)
			r1->m_tree_parent = node2;
		if(l2 != nullptr && l2 != node1)
			l2->m_tree_parent = node1;
		if(r2 != nullptr && r2 != node1)
			r2->m_tree_parent = node1;

		if(node1->m_tree_prev == node2) {
			node1->m_tree_prev = node2->m_tree_prev;
			node2->m_tree_next = node1->m_tree_next;
			node2->m_tree_prev = node1;
			node1->m_tree_next = node2;
			if(node1->m_tree_prev != nullptr)
				node1->m_tree_prev->m_tree_next = node1;
			if(node2->m_tree_next != nullptr)
				node2->m_tree_next->m_tree_prev = node2;
		} else if(node1->m_tree_next == node2) {
			node2->m_tree_prev = node1->m_tree_prev;
			node1->m_tree_next = node2->m_tree_next;
			node1->m_tree_prev = node2;
			node2->m_tree_next = node1;
			if(node2->m_tree_prev != nullptr)
				node2->m_tree_prev->m_tree_next = node2;
			if(node1->m_tree_next != nullptr)
				node1->m_tree_next->m_tree_prev = node1;
		} else {
			std::swap(node1->m_tree_prev, node2->m_tree_prev);
			std::swap(node1->m_tree_next, node2->m_tree_next);
			if(node1->m_tree_prev != nullptr)
				node1->m_tree_prev->m_tree_next = node1;
			if(node1->m_tree_next != nullptr)
				node1->m_tree_next->m_tree_prev = node1;
			if(node2->m_tree_prev != nullptr)
				node2->m_tree_prev->m_tree_next = node2;
			if(node2->m_tree_next != nullptr)
				node2->m_tree_next->m_tree_prev = node2;
		}

	}

	template<typename Compare>
	void TreeInsertAt(SweepEdge *node, Compare &&comp) {
		assert(node != nullptr);

		// insert edge
		if(m_tree_root == nullptr) {
			m_tree_root = node;
			node->m_tree_parent = nullptr;
			node->m_tree_prev = nullptr;
			node->m_tree_next = nullptr;
		} else {
			SweepEdge *current = m_tree_root;
			for( ; ; ) {
				if(comp(current)) {
					if(current->m_tree_right == nullptr) {
						current->m_tree_right = node;
						node->m_tree_parent = current;
						node->m_tree_prev = current;
						node->m_tree_next = current->m_tree_next;
						if(node->m_tree_prev != nullptr)
							node->m_tree_prev->m_tree_next = node;
						if(node->m_tree_next != nullptr)
							node->m_tree_next->m_tree_prev = node;
						break;
					}
					current = current->m_tree_right;
				} else {
					if(current->m_tree_left == nullptr) {
						current->m_tree_left = node;
						node->m_tree_parent = current;
						node->m_tree_prev = current->m_tree_prev;
						node->m_tree_next = current;
						if(node->m_tree_prev != nullptr)
							node->m_tree_prev->m_tree_next = node;
						if(node->m_tree_next != nullptr)
							node->m_tree_next->m_tree_prev = node;
						break;
					}
					current = current->m_tree_left;
				}
			}
		}
		node->m_tree_left = nullptr;
		node->m_tree_right = nullptr;
		node->m_tree_red = true;

		// rebalance
		RebalanceInsert(node);

	}

	void TreeInsertAfter(SweepEdge *node, SweepEdge *after) {
		assert(node != nullptr);
		assert(after != nullptr);

		// insert edge
		if(after->m_tree_right == nullptr) {
			after->m_tree_right = node;
			node->m_tree_parent = after;
		} else {
			SweepEdge *current = after->m_tree_right;
			for( ; ; ) {
				if(current->m_tree_left == nullptr) {
					current->m_tree_left = node;
					node->m_tree_parent = current;
					break;
				}
				current = current->m_tree_left;
			}
		}
		node->m_tree_left = nullptr;
		node->m_tree_right = nullptr;
		node->m_tree_red = true;

		node->m_tree_prev = after;
		node->m_tree_next = after->m_tree_next;
		if(node->m_tree_prev != nullptr)
			node->m_tree_prev->m_tree_next = node;
		if(node->m_tree_next != nullptr)
			node->m_tree_next->m_tree_prev = node;

		// rebalance
		RebalanceInsert(node);

	}

	void TreeRemove(SweepEdge *node) {
		assert(node != nullptr);

		// if the edge has two children, replace it with the leftmost edge in the right subtree
		if(node->m_tree_left != nullptr && node->m_tree_right != nullptr) {
			SweepEdge *current = node->m_tree_right;
			while(current->m_tree_left != nullptr) {
				current = current->m_tree_left;
			}
			TreeSwap(node, current);
		}

		// check the color of the replacement child
		SweepEdge *child = (node->m_tree_right == nullptr)? node->m_tree_left : node->m_tree_right;
		if(!node->m_tree_red) {
			if(child != nullptr && child->m_tree_red) {
				child->m_tree_red = false;
			} else {
				SweepEdge *current = node;
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
			child->m_tree_parent = node->m_tree_parent;
		if(node->m_tree_parent == nullptr) {
			m_tree_root = child;
		} else if(node->m_tree_parent->m_tree_left == node) {
			node->m_tree_parent->m_tree_left = child;
		} else {
			node->m_tree_parent->m_tree_right = child;
		}

		if(node->m_tree_prev != nullptr)
			node->m_tree_prev->m_tree_next = node->m_tree_next;
		if(node->m_tree_next != nullptr)
			node->m_tree_next->m_tree_prev = node->m_tree_prev;


	}

};

}
