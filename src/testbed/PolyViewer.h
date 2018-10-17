#pragma once

#include <QtGui/QtGui>

class PcbViewer : public QWidget {
	Q_OBJECT

private:
	Polygon m_polygon;

public:
	PcbViewer(QWidget* parent);
	virtual ~PcbViewer();

	virtual QSize sizeHint() const override;

protected:
	virtual void paintEvent(QPaintEvent* event) override;

};
