#pragma once

#include "Qt.h"

#include "polymath/PolyMath.h"
#include "testgenerators/TypeConverter.h"

#include <limits>

class VisualizationWrapperBase {
public:
	virtual ~VisualizationWrapperBase() {};
	virtual void Paint(QPainter &painter, double scale) = 0;
};

template<typename T>
class VisualizationWrapper : public VisualizationWrapperBase {

private:
	typedef PolyMath::Vertex<T> Vertex;
	typedef PolyMath::Polygon<T> Polygon;
	typedef PolyMath::Visualization<T> Visualization;

private:
	Polygon m_polygon_input, m_polygon_output;
	Visualization m_visualization;

public:
	VisualizationWrapper() = default;
	VisualizationWrapper(const VisualizationWrapper &other) = default;
	VisualizationWrapper(VisualizationWrapper &&other) = default;

	VisualizationWrapper& operator=(const VisualizationWrapper &other) = default;
	VisualizationWrapper& operator=(VisualizationWrapper &&other) = default;

	void SetPolygonInput(const Polygon &polygon) { m_polygon_input = polygon; }
	void SetPolygonInput(Polygon &&polygon) { m_polygon_input = std::move(polygon); }

	void SetPolygonOutput(const Polygon &polygon) { m_polygon_output = polygon; }
	void SetPolygonOutput(Polygon &&polygon) { m_polygon_output = std::move(polygon); }

	void SetVisualization(const Visualization &visualization) { m_visualization = visualization; }
	void SetVisualization(Visualization &&visualization) { m_visualization = std::move(visualization); }

	static QPainterPath ConvertPolygon(const Polygon &polygon, double mult) {
		QPainterPath multipoly;
		multipoly.setFillRule(Qt::WindingFill);
		for(size_t i = 0; i < polygon.loops.size(); ++i) {
			const Vertex *vertices = polygon.GetLoopVertices(i);
			size_t n = polygon.GetLoopVertexCount(i);
			QPolygonF loop((int(n)));
			QPointF *points = loop.data();
			if(polygon.loops[i].weight < 0) {
				for(size_t j = 0; j < n; ++j) {
					points[n - j - 1] = QPointF(double(vertices[j].x) * mult, double(vertices[j].y) * mult);
				}
			} else {
				for(size_t j = 0; j < n; ++j) {
					points[j] = QPointF(double(vertices[j].x) * mult, double(vertices[j].y) * mult);
				}
			}
			multipoly.addPolygon(loop);
			multipoly.closeSubpath();
		}
		return multipoly;
	}

	virtual void Paint(QPainter &painter, double scale) override {

		double mult = scale / TestGenerators::TypeConverter<T>::ScaleFactor();

		// draw input polygon
		{
			painter.setPen(QPen(QColor(128, 0, 0), 0));
			painter.setBrush(QBrush(QColor(128, 0, 0, 64)));
			painter.drawPath(ConvertPolygon(m_polygon_input, mult));
		}

		// draw output polygon
		{
			painter.setPen(QPen(QColor(255, 255, 0), 0));
			painter.setBrush(QBrush(QColor(128, 128, 0, 64)));
			painter.drawPath(ConvertPolygon(m_polygon_output, mult));
		}

		// draw sweepline
		if(m_visualization.m_has_current_vertex) {
			painter.setPen(QPen(QColor(160, 160, 160), 0));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(QPointF(double(m_visualization.m_current_vertex.x) * mult, -scale), QPointF(double(m_visualization.m_current_vertex.x) * mult, scale));
			painter.drawEllipse(QPointF(double(m_visualization.m_current_vertex.x) * mult, double(m_visualization.m_current_vertex.y) * mult), 3.0, 3.0);
		}

		// draw sweep edges
		painter.setPen(QPen(QColor(128, 255, 128), 0));
		painter.setBrush(Qt::NoBrush);
		for(typename Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

			// draw edge
			painter.drawLine(QPointF(double(a.x) * mult, double(a.y) * mult), QPointF(double(b.x) * mult, double(b.y) * mult));

			// draw intersection with next edge
			if(edge.m_has_intersection) {
				painter.drawEllipse(QPointF(double(edge.m_intersection_vertex.x) * mult, double(edge.m_intersection_vertex.y) * mult), 2.0, 2.0);
			}

		}

		// draw sweep edges intersection with scanline
		if(m_visualization.m_has_current_vertex) {

			size_t num = 0;
			double cy_prev = -std::numeric_limits<double>::max();
			int64_t winding_number = 0;

			int sweep_position = int(lrint(double(m_visualization.m_current_vertex.x) * mult));

			QFont font("Sans");
			font.setPixelSize(8);
			QFontMetrics fm(font);
			painter.setFont(font);

			for(typename Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
				Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

				double div = double(a.x) - double(b.x);
				double cy = (div == 0.0)? double(b.y) : double(b.y) + (double(a.y) - double(b.y)) * (double(m_visualization.m_current_vertex.x) - double(b.x)) / div;
				if(cy_prev != -std::numeric_limits<double>::max()) {
					QString text = QString::number(winding_number);
					int w = fm.horizontalAdvance(text) + 4, h = fm.height() + 4;
					painter.setPen(QPen(QColor(255, 128, 128), 0));
					painter.fillRect(sweep_position - w - 1, int(lrint((cy_prev + cy) / 2.0 * mult)) - h / 2, w, h, QColor(0, 0, 0, 128));
					painter.drawText(sweep_position - w - 1, int(lrint((cy_prev + cy) / 2.0 * mult)) - h / 2, w, h, Qt::AlignHCenter | Qt::AlignVCenter, text);
				}
				{
					QString text = QString::number(++num);
					int w = fm.horizontalAdvance(text) + 4, h = fm.height() + 4;
					painter.setPen(QPen(QColor(128, 255, 128), 0));
					painter.fillRect(sweep_position + 1, int(lrint(cy * mult)) - h / 2, w, h, QColor(0, 0, 0, 128));
					painter.drawText(sweep_position + 1, int(lrint(cy * mult)) - h / 2, w, h, Qt::AlignHCenter | Qt::AlignVCenter, text);
				}
				winding_number = edge.m_winding_number;

				if(cy < cy_prev) {
					painter.setPen(QPen(QColor(0, 255, 255), 0));
					painter.drawLine(QPointF(double(m_visualization.m_current_vertex.x) * mult, cy_prev * mult), QPointF(double(m_visualization.m_current_vertex.x) * mult, cy * mult));
				}
				cy_prev = cy;

			}

		}

		// draw output polygon
		painter.setPen(QPen(QColor(255, 255, 0), 0));
		for(typename Visualization::OutputEdge &edge : m_visualization.m_output_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];
			painter.drawLine(QPointF(double(a.x) * mult, double(a.y) * mult), QPointF(double(b.x) * mult, double(b.y) * mult));
		}

	}

};

class Visualizer : public QWidget {
	Q_OBJECT

public:
	typedef float value_type;
	typedef PolyMath::Vertex<value_type> Vertex;
	typedef PolyMath::Polygon<Vertex> Polygon;
	typedef PolyMath::Visualization<Vertex> Visualization;

private:
	std::unique_ptr<VisualizationWrapperBase> m_wrapper;
	//uint32_t m_decimation_counter;

	bool m_zoom_active;
	double m_zoom_x, m_zoom_y;

public:
	Visualizer(QWidget *parent);
	virtual ~Visualizer() override;

	virtual QSize sizeHint() const override;

	void SetWrapper(std::unique_ptr<VisualizationWrapperBase> &&wrapper);

private:
	void UpdateZoom(int x, int y);

protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void paintEvent(QPaintEvent *event) override;

};
