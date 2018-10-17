#pragma once

#include "Qt.h"

#include "PolyMath.h"

class Visualizer : public QWidget {
	Q_OBJECT

public:
	typedef PolyMath::Vertex<double> Vertex;
	typedef PolyMath::Polygon<Vertex> Polygon;
	typedef PolyMath::Visualization<Vertex> Visualization;

private:
	Polygon m_polygon_input, m_polygon_output;

	bool m_has_visualization;
	Visualization m_visualization;
	uint32_t m_decimation_counter;

	bool m_zoom_active;
	int m_zoom_x, m_zoom_y;

public:
	Visualizer(QWidget *parent);
	virtual ~Visualizer() override;

	virtual QSize sizeHint() const override;

	void SetPolygonInput(const Polygon &polygon);
	void SetPolygonOutput(const Polygon &polygon);
	void SetVisualization(const Visualization &visualization);
	void RemoveVisualization();

protected:
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void paintEvent(QPaintEvent *event) override;

};
