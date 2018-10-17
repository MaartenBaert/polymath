#include "Visualizer.h"
#include "PolyMath.h"

#include <chrono>
#include <thread>

Visualizer::Visualizer(QWidget *parent)
	: QWidget(parent) {

	m_has_visualization = false;
	m_decimation_counter = 0;

	m_zoom_active = false;
	m_zoom_x = 0;
	m_zoom_y = 0;

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMouseTracking(true);

}

Visualizer::~Visualizer() {

}

QSize Visualizer::sizeHint() const {
	return QSize(900, 900);
}

void Visualizer::SetPolygonInput(const Visualizer::Polygon &polygon) {
	m_polygon_input = polygon;
	update();
}

void Visualizer::SetPolygonOutput(const Visualizer::Polygon &polygon) {
	m_polygon_output = polygon;
	update();
}

void Visualizer::SetVisualization(const Visualizer::Visualization &visualization) {
	m_has_visualization = true;
	m_visualization = visualization;
	m_decimation_counter = (m_decimation_counter + 1) % 1;
	if(m_decimation_counter == 0) {
		update();
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		QMessageBox::information(nullptr, "Visualization", "...");
	}
}

void Visualizer::RemoveVisualization() {
	m_has_visualization = false;
	m_visualization = Visualization();
	m_decimation_counter = 0;
	update();
}

void Visualizer::mousePressEvent(QMouseEvent* event) {
	if(event->button() == Qt::LeftButton) {
		m_zoom_active = true;
		m_zoom_x = event->x();
		m_zoom_y = event->y();
		update();
		event->accept();
	}
	if(event->button() == Qt::RightButton) {
		m_zoom_active = false;
		update();
		event->accept();
	}
}

void Visualizer::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);
	QPainter painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(64, 64, 64));

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(width() / 2, height() / 2);
	if(m_zoom_active) {
		painter.scale(10.0, 10.0);
		painter.translate(width() / 2 - m_zoom_x, height() / 2 - m_zoom_y);
	}

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

	if(m_has_visualization) {

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
		for(Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
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
			double cx_prev = -10000.0;
			int64_t winding_number = 0;

			int sweep_position_int = int(lrint(m_visualization.m_current_vertex.y()));

			painter.setFont(font);
			for(Visualization::SweepEdge &edge : m_visualization.m_sweep_edges) {
				Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];

				double div = b.y() - a.y();
				double cx = (div == 0.0)? a.x() : a.x() + (b.x() - a.x()) * (m_visualization.m_current_vertex.y() - a.y()) / div;
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
		for(Visualization::OutputEdge &edge : m_visualization.m_output_edges) {
			Vertex &a = edge.m_edge_vertices[0], &b = edge.m_edge_vertices[1];
			painter.drawLine(QPointF(a.x(), a.y()), QPointF(b.x(), b.y()));
		}

	} else {

		// draw output polygon
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

}
