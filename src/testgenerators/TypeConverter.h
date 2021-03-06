#pragma once

#include "polymath/PolyMath.h"

namespace TestGenerators {

// types used for test generation
typedef PolyMath::Vertex<double> Vertex;
typedef PolyMath::Polygon<double> Polygon;

// type converter base class
template<typename T, typename Enable = void> struct TypeConverterBase;

// for signed integers
template<typename T>
struct TypeConverterBase<T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type> {

	static double ScaleFactor() {
		return double(std::numeric_limits<T>::max() / 2);
	}

	static double Epsilon() {
		return 1.0 / ScaleFactor();
	}

	static T ConvertValueToType(double x) {
		return T(llrint(std::min(std::max(x, -1.0), 1.0) * ScaleFactor()));
	}

	static double ConvertValueFromType(T x) {
		return double(x) * (1.0 / ScaleFactor());
	}

};

// for floating point numbers
template<typename T>
struct TypeConverterBase<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {

	static double ScaleFactor() {
		return 1.0;
	}

	static double Epsilon() {
		return std::numeric_limits<T>::epsilon();
	}

	static T ConvertValueToType(double x) {
		return T(x);
	}

	static double ConvertValueFromType(T x) {
		return double(x);
	}

};

// generic type converter
template<typename T>
struct TypeConverter : TypeConverterBase<T> {

	typedef PolyMath::Vertex<T> VertexType;
	typedef PolyMath::Polygon<T> PolygonType;

	static VertexType ConvertVertexToType(const Vertex &v) {
		return VertexType(TypeConverterBase<T>::ConvertValueToType(v.x), TypeConverterBase<T>::ConvertValueToType(v.y));
	}

	static Vertex ConvertVertexFromType(const VertexType &v) {
		return Vertex(TypeConverterBase<T>::ConvertValueFromType(v.x), TypeConverterBase<T>::ConvertValueFromType(v.y));
	}

	static PolygonType ConvertPolygonToType(const Polygon &input) {
		PolygonType output;
		output.vertices.resize(input.vertices.size());
		output.loops.resize(input.loops.size());
		for(size_t i = 0; i < input.vertices.size(); ++i) {
			output.vertices[i] = ConvertVertexToType(input.vertices[i]);
		}
		for(size_t i = 0; i < input.loops.size(); ++i) {
			output.loops[i].end = input.loops[i].end;
			output.loops[i].weight = input.loops[i].weight;
		}
		return output;
	}

	static Polygon ConvertPolygonFromType(const PolygonType &input) {
		Polygon output;
		output.vertices.resize(input.vertices.size());
		output.loops.resize(input.loops.size());
		for(size_t i = 0; i < input.vertices.size(); ++i) {
			output.vertices[i] = ConvertVertexFromType(input.vertices[i]);
		}
		for(size_t i = 0; i < input.loops.size(); ++i) {
			output.loops[i].end = input.loops[i].end;
			output.loops[i].weight = input.loops[i].weight;
		}
		return output;
	}

};

}
