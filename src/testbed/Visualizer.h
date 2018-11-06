#pragma once

#include "Qt.h"

#include "PolyMath.h"

class VisualizationWrapperBase {
public:
	virtual ~VisualizationWrapperBase() {};
	virtual void Paint(QPainter &painter) = 0;
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

	virtual void Paint(QPainter &painter) override {

		QFont font("Sans");
		font.setPixelSize(8);
		QFontMetrics fm(font);

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
					points[j] = QPointF(vertices[j].x(), vertices[j].y());
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
					points[j] = QPointF(vertices[j].x(), vertices[j].y());
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
			painter.drawLine(QPointF(-10000.0, m_visualization.m_current_vertex.y()), QPointF(10000.0, m_visualization.m_current_vertex.y()));
			painter.drawEllipse(QPointF(m_visualization.m_current_vertex.x(), m_visualization.m_current_vertex.y()), 3.0, 3.0);
		}

		// draw sweep edges
		painter.setPen(QPen(QColor(128, 255, 128), 0));
		painter.setBrush(Qt::NoBrush);
		for(typename Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

			// draw edge
			painter.drawLine(QPointF(a.x(), a.y()), QPointF(b.x(), b.y()));

			// draw intersection with next edge
			if(edge.m_has_intersection) {
				painter.drawEllipse(QPointF(edge.m_intersection_vertex.x(), edge.m_intersection_vertex.y()), 2.0, 2.0);
			}

		}

		// draw sweep edges intersection with scanline
		if(m_visualization.m_has_current_vertex) {

			size_t num = 0;
			F cx_prev = -10000.0;
			int64_t winding_number = 0;

			int sweep_position_int = int(lrint(m_visualization.m_current_vertex.y()));

			painter.setFont(font);
			for(typename Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
				Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

				F div = b.y() - a.y();
				F cx = (div == 0.0)? a.x() : a.x() + (b.x() - a.x()) * (m_visualization.m_current_vertex.y() - a.y()) / div;
				{
					QString text = QString::number(winding_number);
					int w = fm.width(text) + 4, h = fm.height() + 4;
					painter.setPen(QPen(QColor(255, 128, 128), 0));
					painter.fillRect(int(lrint((cx_prev + cx) / 2.0)) - w / 2, sweep_position_int - h - 1, w, h, QColor(0, 0, 0, 128));
					painter.drawText(int(lrint((cx_prev + cx) / 2.0)) - w / 2, sweep_position_int - h - 1, w, h, Qt::AlignHCenter | Qt::AlignVCenter, text);
				}
				{
					QString text = QString::number(++num);
					int w = fm.width(text) + 4, h = fm.height() + 4;
					painter.setPen(QPen(QColor(128, 255, 128), 0));
					painter.fillRect(int(lrint(cx)) - w / 2, sweep_position_int + 1, w, h, QColor(0, 0, 0, 128));
					painter.drawText(int(lrint(cx)) - w / 2, sweep_position_int + 1, w, h, Qt::AlignHCenter | Qt::AlignVCenter, text);
				}
				winding_number = edge.m_winding_number;

				if(cx < cx_prev) {
					painter.setPen(QPen(QColor(0, 255, 255), 0));
					painter.drawLine(QPointF(cx_prev, m_visualization.m_current_vertex.y()), QPointF(cx, m_visualization.m_current_vertex.y()));
				}
				cx_prev = cx;

			}

		}

		// draw output polygon
		painter.setPen(QPen(QColor(255, 255, 0), 0));
		for(typename Visualization::OutputEdge &edge : m_visualization.m_output_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];
			painter.drawLine(QPointF(a.x(), a.y()), QPointF(b.x(), b.y()));
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
