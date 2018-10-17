#pragma once

#include "Qt.h"
#include "Visualizer.h"

#include "PolyMath.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	typedef PolyMath::Vertex<double> Vertex;
	typedef PolyMath::Polygon<Vertex> Polygon;
	typedef PolyMath::Visualization<Vertex> Visualization;

private:
	Polygon m_polygon;

	QSpinBox *m_spinbox_seed;
	QSpinBox *m_spinbox_size;
	QDoubleSpinBox *m_spinbox_angle;
	Visualizer *m_visual;

public:
	MainWindow();
	~MainWindow();

private:
	void MakeTest(uint64_t seed = 0, uint32_t grid_size = 10, double grid_angle = 20.0);

public slots:
	void OnTestChanged();
	void OnButtonSimplify();
	void OnButtonBenchmark();
	void OnButtonEdgeCases();

};
