#pragma once

#include "Qt.h"

#include "PolyMath.h"

class VisualizationWrapperBase {
public:
	virtual ~VisualizationWrapperBase() {};
	virtual void Paint(QPainter &painter, double scale) = 0;
};

template<typename F>
class VisualizationWrapper : public VisualizationWrapperBase {

private:
	typedef PolyMath::Vertex<F> Vertex;
	typedef PolyMath::Polygon<Vertex> Polygon;
	typedef PolyMath::Visualization<Vertex> Visualization;

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

	virtual void Paint(QPainter &painter, double scale) override {

		double limit = scale;

		// draw input polygon
		{
			painter.setPen(QPen(QColor(128, 0, 0), 0));
			painter.setBrush(QBrush(QColor(128, 0, 0, 64)));
			QPainterPath multipoly;
			multipoly.setFillRule(Qt::OddEvenFill); // Qt::WindingFill
			for(size_t i = 0; i < m_polygon_input.GetLoopCount(); ++i) {
				Vertex *vertices = m_polygon_input.GetLoopVertices(i);
				size_t n = m_polygon_input.GetLoopVertexCount(i);
				QPolygonF poly((int(n)));
				QPointF *points = poly.data();
				for(size_t j = 0; j < n; ++j) {
					points[j] = QPointF(double(vertices[j].x()) * scale, double(vertices[j].y()) * scale);
				}
				multipoly.addPolygon(poly);
				multipoly.closeSubpath();
			}
			painter.drawPath(multipoly);
		}

		// draw output polygon
		{
			painter.setPen(QPen(QColor(255, 255, 0), 0));
			painter.setBrush(QBrush(QColor(128, 128, 0, 64)));
			QPainterPath multipoly;
			multipoly.setFillRule(Qt::OddEvenFill); // Qt::WindingFill
			for(size_t i = 0; i < m_polygon_output.GetLoopCount(); ++i) {
				Vertex *vertices = m_polygon_output.GetLoopVertices(i);
				size_t n = m_polygon_output.GetLoopVertexCount(i);
				QPolygonF poly((int(n)));
				QPointF *points = poly.data();
				for(size_t j = 0; j < n; ++j) {
					points[j] = QPointF(double(vertices[j].x()) * scale, double(vertices[j].y()) * scale);
				}
				multipoly.addPolygon(poly);
				multipoly.closeSubpath();
			}
			painter.drawPath(multipoly);
		}

		// draw sweepline
		if(m_visualization.m_has_current_vertex) {
			painter.setPen(QPen(QColor(160, 160, 160), 0));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(QPointF(double(m_visualization.m_current_vertex.x()) * scale, -limit), QPointF(double(m_visualization.m_current_vertex.x()) * scale, limit));
			painter.drawEllipse(QPointF(double(m_visualization.m_current_vertex.x()) * scale, double(m_visualization.m_current_vertex.y()) * scale), 3.0, 3.0);
		}

		// draw sweep edges
		painter.setPen(QPen(QColor(128, 255, 128), 0));
		painter.setBrush(Qt::NoBrush);
		for(typename Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

			// draw edge
			painter.drawLine(QPointF(double(a.x()) * scale, double(a.y()) * scale), QPointF(double(b.x()) * scale, double(b.y()) * scale));

			// draw intersection with next edge
			if(edge.m_has_intersection) {
				painter.drawEllipse(QPointF(double(edge.m_intersection_vertex.x()) * scale, double(edge.m_intersection_vertex.y()) * scale), 2.0, 2.0);
			}

		}

		// draw sweep edges intersection with scanline
		if(m_visualization.m_has_current_vertex) {

			size_t num = 0;
			double cy_prev = -limit;
			int64_t winding_number = 0;

			int sweep_position = int(lrint(double(m_visualization.m_current_vertex.x()) * scale));

			QFont font("Sans");
			font.setPixelSize(8);
			QFontMetrics fm(font);
			painter.setFont(font);

			for(typename Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
				Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

				double div = double(b.x()) - double(a.x());
				double cy = (div == 0.0)? double(a.y()) : double(a.y()) + (double(b.y()) - double(a.y())) * (double(m_visualization.m_current_vertex.x()) - double(a.x())) / div;
				{
					QString text = QString::number(winding_number);
					int w = fm.width(text) + 4, h = fm.height() + 4;
					painter.setPen(QPen(QColor(255, 128, 128), 0));
					painter.fillRect(sweep_position - w - 1, int(lrint((cy_prev + cy) / 2.0 * scale)) - h / 2, w, h, QColor(0, 0, 0, 128));
					painter.drawText(sweep_position - w - 1, int(lrint((cy_prev + cy) / 2.0 * scale)) - h / 2, w, h, Qt::AlignHCenter | Qt::AlignVCenter, text);
				}
				{
					QString text = QString::number(++num);
					int w = fm.width(text) + 4, h = fm.height() + 4;
					painter.setPen(QPen(QColor(128, 255, 128), 0));
					painter.fillRect(sweep_position + 1, int(lrint(cy * scale)) - h / 2, w, h, QColor(0, 0, 0, 128));
					painter.drawText(sweep_position + 1, int(lrint(cy * scale)) - h / 2, w, h, Qt::AlignHCenter | Qt::AlignVCenter, text);
				}
				winding_number = edge.m_winding_number;

				if(cy < cy_prev) {
					painter.setPen(QPen(QColor(0, 255, 255), 0));
					painter.drawLine(QPointF(double(m_visualization.m_current_vertex.x()) * scale, cy_prev * scale), QPointF(double(m_visualization.m_current_vertex.x()) * scale, cy * scale));
				}
				cy_prev = cy;

			}

		}

		// draw output polygon
		painter.setPen(QPen(QColor(255, 255, 0), 0));
		for(typename Visualization::OutputEdge &edge : m_visualization.m_output_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];
			painter.drawLine(QPointF(double(a.x()) * scale, double(a.y()) * scale), QPointF(double(b.x()) * scale, double(b.y()) * scale));
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
	int m_zoom_x, m_zoom_y;

public:
	Visualizer(QWidget *parent);
	virtual ~Visualizer() override;

	virtual QSize sizeHint() const override;

	void SetWrapper(std::unique_ptr<VisualizationWrapperBase> &&wrapper);

protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void paintEvent(QPaintEvent *event) override;

};