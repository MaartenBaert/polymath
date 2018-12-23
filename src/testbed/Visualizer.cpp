#include "Visualizer.h"

#include "polymath/PolyMath.h"

#include <chrono>
#include <thread>

/*
	m_decimation_counter = (m_decimation_counter + 1) % 1;
	if(m_decimation_counter == 0) {
		update();
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		QMessageBox::information(nullptr, "Visualization", "...");
	}
*/

Visualizer::Visualizer(QWidget *parent)
	: QWidget(parent) {

	//m_decimation_counter = 0;

	m_zoom_active = false;
	m_zoom_x = 0;
	m_zoom_y = 0;

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

}

Visualizer::~Visualizer() {

}

QSize Visualizer::sizeHint() const {
	return QSize(600, 600);
}

void Visualizer::SetWrapper(std::unique_ptr<VisualizationWrapperBase> &&wrapper) {
	m_wrapper = std::move(wrapper);
	update();
}

void Visualizer::UpdateZoom(int x, int y) {
	double scale = double(std::min(width() / 2, height() / 2));
	m_zoom_active = true;
	m_zoom_x = double(x - width() / 2) / scale;
	m_zoom_y = double(y - height() / 2) / scale;
	update();
}

void Visualizer::mousePressEvent(QMouseEvent *event) {
	if(event->button() == Qt::LeftButton) {
		UpdateZoom(event->x(), event->y());
		event->accept();
	}
	if(event->button() == Qt::RightButton) {
		m_zoom_active = false;
		update();
		event->accept();
	}
}

void Visualizer::mouseMoveEvent(QMouseEvent *event) {
	if(event->buttons() & Qt::LeftButton) {
		UpdateZoom(event->x(), event->y());
		event->accept();
	}
}

void Visualizer::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);
	QPainter painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(64, 64, 64));

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(width() / 2, height() / 2);

	double scale = double(std::min(width() / 2, height() / 2));
	if(m_zoom_active) {
		scale *= 10.0;
		painter.translate(-scale * m_zoom_x, -scale * m_zoom_y);
	}
	if(m_wrapper != nullptr) {
		m_wrapper->Paint(painter, scale);
	}

	painter.resetTransform();
	painter.setFont(font());
	painter.setPen(QColor(192, 192, 192));
	painter.drawText(5, 5, width() - 10, height() - 10, Qt::AlignLeft | Qt::AlignBottom, "Left-click to zoom in\nRight-click to reset view");

}
