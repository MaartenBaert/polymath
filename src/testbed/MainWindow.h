#pragma once

#include "Qt.h"
#include "Visualizer.h"

#include "PolyMath.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

private:
	enum Type {
		TYPE_F32,
		TYPE_F64,
	};

private:
	typedef PolyMath::Vertex<double> Vertex;
	typedef PolyMath::Polygon<Vertex> Polygon;
	typedef PolyMath::Visualization<Vertex> Visualization;

private:
	Polygon m_polygon;

	QComboBox *m_combobox_type;
	QSpinBox *m_spinbox_seed;
	QSpinBox *m_spinbox_size;
	QDoubleSpinBox *m_spinbox_angle;
	Visualizer *m_visualizer;

public:
	MainWindow();
	~MainWindow();

public slots:
	void OnTestChanged();
	void OnButtonVisualize();
	void OnButtonBenchmark();
	void OnButtonEdgeCases();

};
