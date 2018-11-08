#include "Visualizer.h"
#include "PolyMath.h"

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
	setMouseTracking(true);

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

	double scale = double(std::min(width() / 2, height() / 2));
	if(m_zoom_active) {
		scale *= 10.0;
		painter.translate((width() / 2 - m_zoom_x) * 10, (height() / 2 - m_zoom_y) * 10);
	}

	if(m_wrapper != nullptr) {
		m_wrapper->Paint(painter, scale);
	}

}
