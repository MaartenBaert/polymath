#include "PcbViewer.h"

#include <cassert>
#include <random>

PcbViewer::PcbViewer(QWidget* parent)
	: QWidget(parent) {

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMouseTracking(true);



}

PcbViewer::~PcbViewer() {

}

QSize PcbViewer::sizeHint() const {
	return QSize(800, 800);
}

void PcbViewer::paintEvent(QPaintEvent* event) {
	Q_UNUSED(event);
	QPainter painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(64, 64, 64));

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(width() / 2, height() / 2);
	//painter.scale(2.0, 2.0);

	painter.setPen(QPen(QColor(255, 0, 0), 0));
	for(size_t i = 0; i < m_polygon.GetLoops(); ++i) {
		for(size_t j = 0; j < m_polygon[i].GetVertices(); ++j) {
			size_t k = (j + 1) % m_polygon[i].GetVertices();
			Vertex a = m_polygon[i][j], b = m_polygon[i][k];
			painter.drawLine(a.x, a.y, b.x, b.y);

		}
	}

}
