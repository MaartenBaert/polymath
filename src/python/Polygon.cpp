#include "polymath/PolyMath.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <sstream>

typedef PolyMath::Vertex<int64_t> Vertex_I64;
typedef PolyMath::Polygon<int64_t> Polygon_I64;

template<class Polygon>
Polygon PolygonImport(const std::vector<std::pair<pybind11::array_t<typename Polygon::VertexType::ValueType>, typename Polygon::WindingWeightType>> &loops) {
	Polygon p;
	for(auto &loop : loops) {
		auto data = loop.first.template unchecked<2>();
		if(data.shape(1) != 2)
			throw std::runtime_error("Incorrect shape");
		for(size_t i = 0; i < size_t(data.shape(0)); ++i) {
			p.AddVertex(Vertex_I64(data(i, 0), data(i, 1)));
		}
		p.AddLoopEnd(loop.second);
	}
	return p;
}

template<class Polygon>
std::vector<std::pair<pybind11::array_t<typename Polygon::VertexType::ValueType>, typename Polygon::WindingWeightType>> PolygonExport(const Polygon& p) {
	std::vector<std::pair<pybind11::array_t<typename Polygon::VertexType::ValueType>, typename Polygon::WindingWeightType>> loops;
	for(size_t i = 0; i < p.loops.size(); ++i) {
		const typename Polygon::VertexType *v = p.GetLoopVertices(i);
		size_t n = p.GetLoopVertexCount(i);
		pybind11::array_t<typename Polygon::VertexType::ValueType> arr(std::array<size_t, 2>{n, 2});
		auto data = arr.mutable_unchecked();
		for(size_t i = 0; i < size_t(data.shape(0)); ++i) {
			data(i, 0) = v[i].x;
			data(i, 1) = v[i].y;
		}
		loops.emplace_back(std::move(arr), p.loops[i].weight);
	}
	return loops;
}

PYBIND11_MODULE(polymath, m) {
	m.doc() = "PolyMath python bindings";

	pybind11::class_<Vertex_I64>(m, "Vertex_I64")
			.def(pybind11::init<>())
			.def(pybind11::init<const Vertex_I64&>())
			.def(pybind11::init<Vertex_I64::ValueType, Vertex_I64::ValueType>())
			.def("copy", [](const Vertex_I64 &v) -> Vertex_I64 { return v; })
			.def_readwrite("x", &Vertex_I64::x)
			.def_readwrite("y", &Vertex_I64::y)
			.def("__str__", [](Vertex_I64 &p) -> std::string { std::ostringstream ss; ss << p; return ss.str(); });

	pybind11::class_<Polygon_I64>(m, "Polygon_I64")
			.def(pybind11::init<>())
			.def(pybind11::init<const Polygon_I64&>())
			.def(pybind11::init(&PolygonImport<Polygon_I64>))
			.def("copy", [](const Polygon_I64 &p) -> Polygon_I64 { return p; })
			.def("export", &PolygonExport<Polygon_I64>)
			.def(pybind11::self + pybind11::self)
			.def(pybind11::self - pybind11::self)
			.def(pybind11::self += pybind11::self)
			.def(pybind11::self -= pybind11::self)
			.def("clear", &Polygon_I64::Clear)
			.def("add_vertex", &Polygon_I64::AddVertex)
			.def("add_loop_end", &Polygon_I64::AddLoopEnd)
			.def("__str__", [](Polygon_I64 &p) -> std::string { std::ostringstream ss; ss << p; return ss.str(); });

	m.def("PolygonSimplify_NonZero", &PolyMath::PolygonSimplify_NonZero<int64_t, int32_t>);
	m.def("PolygonSimplify_EvenOdd", &PolyMath::PolygonSimplify_EvenOdd<int64_t, int32_t>);
	m.def("PolygonSimplify_Positive", &PolyMath::PolygonSimplify_Positive<int64_t, int32_t>);
	m.def("PolygonSimplify_Negative", &PolyMath::PolygonSimplify_Negative<int64_t, int32_t>);

}
