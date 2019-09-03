#include "polymath/PolyMath.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <sstream>

template<typename T>
PolyMath::Polygon<T> PolygonImport(const std::vector<std::pair<pybind11::array_t<T>, PolyMath::default_winding_t>> &loops) {
	PolyMath::Polygon<T> p;
	for(auto &loop : loops) {
		auto data = loop.first.template unchecked<2>();
		if(data.shape(1) != 2)
			throw std::runtime_error("Incorrect shape");
		for(size_t i = 0; i < size_t(data.shape(0)); ++i) {
			p.AddVertex(PolyMath::Vertex<T>(data(i, 0), data(i, 1)));
		}
		p.AddLoopEnd(loop.second);
	}
	return p;
}

template<typename T>
std::vector<std::pair<pybind11::array_t<T>, PolyMath::default_winding_t>> PolygonExport(const PolyMath::Polygon<T>& p) {
	std::vector<std::pair<pybind11::array_t<T>, PolyMath::default_winding_t>> loops;
	for(size_t i = 0; i < p.loops.size(); ++i) {
		const PolyMath::Vertex<T> *v = p.GetLoopVertices(i);
		size_t n = p.GetLoopVertexCount(i);
		pybind11::array_t<T> arr(std::array<size_t, 2>{n, 2});
		auto data = arr.mutable_unchecked();
		for(size_t i = 0; i < size_t(data.shape(0)); ++i) {
			data(i, 0) = v[i].x;
			data(i, 1) = v[i].y;
		}
		loops.emplace_back(std::move(arr), p.loops[i].weight);
	}
	return loops;
}

template<typename T>
void RegisterPolygon(pybind11::module &m, const char *name) {
	typedef PolyMath::Vertex<T> Vertex;
	typedef PolyMath::Polygon<T> Polygon;
	pybind11::class_<Polygon>(m, name)
			.def(pybind11::init<>())
			.def(pybind11::init<const Polygon&>())
			.def(pybind11::init(&PolygonImport<T>))
			.def("copy", [](const Polygon &p) -> Polygon { return p; })
			.def("export", &PolygonExport<T>)
			.def(pybind11::self + pybind11::self)
			.def(pybind11::self - pybind11::self)
			.def(pybind11::self += pybind11::self)
			.def(pybind11::self -= pybind11::self)
			.def("clear", &Polygon::Clear)
			.def("add_vertex", [](Polygon &p, T x, T y) { p.AddVertex(Vertex(x, y)); })
			.def("add_loop_end", &Polygon::AddLoopEnd)
			.def("__str__", [](Polygon &p) -> std::string { std::ostringstream ss; ss << p; return ss.str(); });
	m.def("make_simple", [](const Polygon &p, PolyMath::WindingRule winding_rule) {
		typedef PolyMath::OutputPolicy_Simple<T> OutputPolicy;
		typedef PolyMath::WindingPolicy_Dynamic<> WindingPolicy;
		PolyMath::SweepEngine<T, OutputPolicy, WindingPolicy> engine(p, OutputPolicy(), WindingPolicy(winding_rule));
		engine.Process();
		return engine.Result();
	});
	m.def("make_keyhole", [](const Polygon &p, PolyMath::WindingRule winding_rule) {
		typedef PolyMath::OutputPolicy_Keyhole<T> OutputPolicy;
		typedef PolyMath::WindingPolicy_Dynamic<> WindingPolicy;
		PolyMath::SweepEngine<T, OutputPolicy, WindingPolicy> engine(p, OutputPolicy(), WindingPolicy(winding_rule));
		engine.Process();
		return engine.Result();
	});
}

PYBIND11_MODULE(polymath, m) {
	m.doc() = "PolyMath python bindings";

	pybind11::enum_<PolyMath::WindingRule>(m, "WindingRule")
			.value("NONZERO", PolyMath::WINDINGRULE_NONZERO)
			.value("EVENODD", PolyMath::WINDINGRULE_EVENODD)
			.value("POSITIVE", PolyMath::WINDINGRULE_POSITIVE)
			.value("NEGATIVE", PolyMath::WINDINGRULE_NEGATIVE)
			.export_values();

	RegisterPolygon<int32_t>(m, "Polygon_I32");
	RegisterPolygon<int64_t>(m, "Polygon_I64");
	RegisterPolygon<float>(m, "Polygon_F32");
#ifdef __SIZEOF_FLOAT128__
	RegisterPolygon<double>(m, "Polygon_F64");
#endif

}
