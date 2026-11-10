# PolyMath design notes

This document describes how PolyMath works internally and what to keep in mind when using or modifying it. For a short overview and benchmark results, see the README.

## Overview

PolyMath performs boolean operations, simplification and decomposition (keyhole, monotone, triangles) of polygons with a single sweep line pass. The core is `SweepEngine`, which is parametrized with policies:

```cpp
PolyMath::SweepEngine<T, OutputPolicy, WindingPolicy, SweepTree> engine(input_polygon);
engine.Process();
PolyMath::Polygon<T> result = engine.Result();
```

| Parameter | Options |
|---|---|
| `T` (coordinate type) | `int8_t`, `int16_t`, `int32_t`, `int64_t`, `float`, `double` (see [Numerical engines](#numerical-engines)) |
| `OutputPolicy` | `OutputPolicy_Simple` (outer loops and holes), `OutputPolicy_Keyhole` (holes joined to their outer loop by cuts, so every polygon is a single loop), `OutputPolicy_Monotone` (decomposition into monotone polygons), `OutputPolicy_Triangles` (triangulation) |
| `WindingPolicy` | `WindingPolicy_NonZero`, `WindingPolicy_EvenOdd`, `WindingPolicy_Positive`, `WindingPolicy_Negative`, or `WindingPolicy_Dynamic` (rule chosen at runtime) |
| `SweepTree` | `SweepTree_Basic` or `SweepTree_Basic2` (see [Data structures](#data-structures)) |

## Polygons, orientation and winding numbers

A `Polygon` is a list of vertices plus a list of loops. Each loop stores the index of its last vertex plus one, and a **winding weight**. The winding number of a point is the sum of the weights of all loops that enclose it, where a loop counts positively or negatively depending on its orientation. The winding policy then decides which winding numbers are "inside".

Boolean operations are expressed through weights and rules. Examples:

- **Union**: all loops weight +1, rule *positive*.
- **Difference A − B**: loops of A weight +1, loops of B weight −1, rule *positive*. This also works for holes: a hole can have the same orientation as its outer loop with weight −1.
- **Intersection of A and B**: all loops weight +1, and a rule that requires winding number ≥ 2 (not one of the built-in policies yet).
- **Simplification** (removing self-intersections and redundant edges): any input, rule *nonzero* or *even-odd*.

**Handedness doesn't matter.** PolyMath never assumes which way the axes point. With the Y axis pointing **down**, a loop that is **clockwise** with weight +1 adds +1 to the winding number inside it. With the Y axis pointing **up**, the same loop appears **counter-clockwise**, and everything works the same. Reversing a loop's orientation has the same effect as negating its weight. Applications can therefore use whichever convention they like, as long as they are consistent.

**Output orientation.** All output policies use the same convention as the input. Output loops have weight +1, outer loops have the positive orientation and holes the opposite one, and triangles all have the positive orientation. This means the output of one operation can be used directly as input for the next one with the same winding rule. This is verified by the unit tests (see [Testing](#testing)).

## The sweep algorithm

The algorithm is based on the Bentley–Ottmann line segment intersection algorithm. It is extended to handle polygons instead of loose line segments, to keep track of winding numbers, and to generate output polygons through the output policy.

- **Time complexity:** O((n + k) log n)
- **Space complexity:** O(n + p)

where n is the number of input vertices, k the number of intersections and p the number of output vertices.

**Sweep direction.** The sweep line is vertical and moves in the +X direction. Vertices are processed in order of increasing X, then increasing Y, with a consistent tie breaker (the vertex address, which is deterministic because all vertices are stored in one array). Sorting by Y as a secondary key avoids a lot of special cases for vertical edges.

**Events.** Each input vertex is classified by the direction of its two edges:

- **start vertex**: both edges go forward (to the right). Two new edges are inserted into the tree.
- **middle vertex**: one edge comes in from the left, one continues to the right. The edge in the tree is replaced.
- **stop vertex**: both edges come in from the left. Two edges are removed from the tree.

**Intersections** between neighbouring edges in the tree are calculated as soon as edges become neighbours, and stored in a heap. Before each input vertex is processed, all pending intersections with an X coordinate less than or equal to that vertex are processed first. Processing an intersection swaps the two edges in the tree, updates their winding numbers, recalculates intersections with the new neighbours, and passes the result to the output policy.

**Winding numbers** are stored per edge: each edge stores the winding number of the region on one side of it. When edges are inserted or swapped, the winding numbers are updated incrementally from the neighbouring edge and the edge's winding weight. The winding policy evaluates the regions on both sides of an edge. The edge is part of the output if exactly one side is inside.

## Data structures

The algorithm needs several data structures that all reference each other. This rules out standard library containers. They are all implemented directly on the vertex and edge structs, which keeps the number of allocations and cache misses low. This is the main reason PolyMath is fast:

- **Sorted vertex array**: all input vertices, sorted in sweep order (see above). Each vertex links to the previous and next vertex in its loop.
- **Sweep edges**: allocated in blocks and recycled through a free list. A sweep edge exists only while it intersects the sweep line.
- **Search tree**: a red-black tree of the edges that currently intersect the sweep line, ordered along the sweep line. The ordering changes as the sweep line moves, but only at intersections, which are handled explicitly by swapping two nodes. `SweepTree_Basic2` additionally threads the nodes into a doubly linked list (previous/next pointers), which makes neighbour lookups O(1).
- **Intersection heap**: a binary min-heap keyed on the X coordinate of the intersection. Each edge has at most one pending intersection (with its neighbour on one side), and stores its own index in the heap, so intersections can be removed in O(log n) when edges stop being neighbours.

## Precision and edge cases

PolyMath does **not** produce exact results. Intersections of line segments are usually not exactly representable in the coordinate type. PolyMath instead tries to produce results that are accurate up to machine precision. Due to compounding errors the actual error can occasionally be slightly larger, but not by much. Whenever an exact result is possible, PolyMath tries to produce it exactly.

Critical calculations (orientation tests and intersections) use increased precision internally, and the algorithms are designed to tolerate rounding errors without breaking down:

- **Intersections vs. vertices.** Ideally all events (vertices and intersections) would be sorted by X and then by Y. That doesn't work for intersections, because they aren't calculated exactly. Intersections are therefore ordered by X only, and are always processed before an input vertex with the same X. This is safe because an intersection can only be in the heap while both edges are in the tree, and only if it is the first unprocessed intersection of at least one of the two edges.
- **Inserting a start vertex into the tree** uses the rule "edge < vertex" (an orientation test, `CompareEdgeVertex`). For an edge that is parallel to the sweep line, this test is always false. That is correct: the inserted vertex then lies between the two vertices of the parallel edge, so it lies on the edge and either order is valid. Together with the rule that intersections are handled first, this keeps the tree consistent in the presence of vertical edges.
- **Edges that should be neighbours but aren't.** Because of rounding errors, two edges that meet at a stop vertex are not always adjacent in the tree; other edges can be "stuck" between them. The engine searches in both directions to find the other edge, and forces intersections for any edges in between before removing the pair.
- **Intersections are rounded** to the coordinate type when they are processed (they are calculated in double precision). The output is therefore topologically consistent, but after rounding it is not guaranteed to be free of self-intersections. See [Future work](#future-work).

## Numerical engines

The numerical engine (`NumericalEngine<T>`) provides the orientation test and the intersection calculation for each coordinate type, using wider types where needed:

| Coordinate type | Internal precision | Notes |
|---|---|---|
| `int8_t`, `int16_t` | 2× and 4× native integers | |
| `int32_t` | `int64_t` and wider | |
| `int64_t` | 128/256-bit integers (`widemath`) | Fast inline assembly on x86_64 (`WIDEMATH_USE_ASM`), plain C fallback elsewhere. |
| `float` | `double` | Recommended floating point type. |
| `double` | `__float128` | Software-emulated, very slow, not supported by all compilers. |

The **integer engine** is more predictable than the floating point engine: its rounding errors are deterministic and don't depend on hardware or compiler options. It is not necessarily more accurate. The **floating point engine** is the most convenient for graphics, but results may differ slightly between platforms.

## Testing

The unit tests (`polymath-unittests`, also registered with `ctest`) contain three kinds of tests for the sweep engine and output policies:

- **Golden tests** (`TestGolden.cpp`): the output of every output policy, for every winding rule, with the `int32_t` and `int64_t` engines, is compared with hashes of known-good output for a fixed set of generated inputs. The hashes use a canonical form of the output, so they don't depend on the start vertex of each loop or the order of the loops, only on the geometry and the orientation. Floating point engines are not included because their results may differ between platforms. If the output changes intentionally, the golden data can be regenerated with `polymath-unittests "Generate golden data"` (see the comment in `TestGolden.cpp`).
- **Winding number probes** (`TestOutput.cpp`): random points that are not close to any edge are tested against the input and the output. The winding number of the output must be exactly 1 where the input is inside according to the winding rule, and 0 elsewhere. This checks correctness and orientation of all output policies with all winding rules and engines (including `float`), independently of the golden data.
- **Chaining tests** (`TestOutput.cpp`): the output of each policy is fed back into the engine with the same winding rule, which must produce the same area.

The test inputs come from the test generators, which must produce the same polygons on every platform. Random numbers must therefore never be generated in function arguments, since the evaluation order of function arguments is unspecified (GCC and Clang use different orders).

**Internal consistency checks** can be enabled by defining `POLYMATH_VERIFY=1`, e.g. with `-DCMAKE_CXX_FLAGS=-DPOLYMATH_VERIFY=1`. This verifies the heap and the winding numbers after every step, and checks that exactly those edges that separate inside from outside have an output edge. It is very slow, but useful when changing the sweep engine or the output policies.

## Future work

Planned features, mostly driven by AlterPCB:

- **Snap rounding**, so that the output is guaranteed to be free of self-intersections after rounding to the coordinate type, and consecutive boolean operations behave sensibly. It still needs to be worked out how to integrate this into the sweep engine.
- **Simplification of degenerate output**: shapes that touch exactly or within a rounding tolerance, zero-area slivers and spikes. Probably a post-processing step after the sweep engine has deduplicated output vertices (but not output edges). It could be combined with snap rounding.
- **Minkowski sum with a convex polygon**, as the basis for both stroking (paths with a pen shape) and offsetting. An algorithm with acceptable worst-case complexity is still needed. For PCB use, circles are approximated with circumscribed polygons (the tangent rule), with the number of vertices a multiple of 8.
- **Connectivity graph generation**: which output polygons touch or overlap.
- `PolygonPointEdgeDistance` calculates squared distances in the coordinate type, which overflows for integer coordinates that use a significant part of the range. It should use the wider numerical engine types.
- An **intersection winding policy** (winding number ≥ 2) and other combined rules.
