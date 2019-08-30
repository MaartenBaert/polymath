#pragma once

#include "Qt.h"
#include "Visualizer.h"

#include "polymath/PolyMath.h"

// workaround for QTBUG-72647
class QStackedLayoutFixed : public QStackedLayout {
	Q_OBJECT

public:
	inline QStackedLayoutFixed() {}
	inline explicit QStackedLayoutFixed(QWidget *parent) : QStackedLayout(parent) {}
	inline explicit QStackedLayoutFixed(QLayout *parentLayout) : QStackedLayout(parentLayout) {}
	~QStackedLayoutFixed();

	virtual Qt::Orientations expandingDirections() const override;

};

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	enum Type {
		TYPE_I8,
		TYPE_I16,
		TYPE_I32,
		TYPE_I64,
		TYPE_F32,
		TYPE_F64,
	};
	enum Output {
		OUTPUT_SIMPLE,
		OUTPUT_MONOTONE,
	};
	enum TestType {
		TESTTYPE_DUALGRID,
		TESTTYPE_ORTHOGONAL,
		TESTTYPE_EDGECASES,
		TESTTYPE_STAR,
	};

private:
	typedef PolyMath::Vertex<double> Vertex;
	typedef PolyMath::Polygon<double> Polygon;
	typedef PolyMath::Visualization<double> Visualization;

private:
	Polygon m_polygon;

	// settings
	QSpinBox *m_settings_seed_spinbox;
	QComboBox *m_settings_type_combobox;
	QComboBox *m_settings_output_combobox;
	//QCheckBox *m_settings_fusion_checkbox;

	// test
	QComboBox *m_test_combobox;
	QStackedLayoutFixed *m_test_stackedlayout;

	// test: dual grid
	QComboBox *m_test_dualgrid_type_combobox;
	QSpinBox *m_test_dualgrid_size_spinbox;
	QDoubleSpinBox *m_test_dualgrid_angle_spinbox;
	QCheckBox *m_test_dualgrid_holes_checkbox;

	// test: orthogonal
	QSpinBox *m_test_orthogonal_lines_spinbox;
	QSpinBox *m_test_orthogonal_polygons_spinbox;

	// test: edge cases
	QSpinBox *m_test_edgecases_lines_spinbox;
	QSpinBox *m_test_edgecases_polygons_spinbox;
	QDoubleSpinBox *m_test_edgecases_epsilon_spinbox;

	// test: star
	QSpinBox *m_test_star_points_spinbox;
	QDoubleSpinBox *m_test_star_angle_spinbox;

	Visualizer *m_visualizer;

public:
	MainWindow();
	~MainWindow();

private:
	static double GetEpsilon(Type type);

public slots:
	void OnTestChanged();
	void OnButtonVisualize();
	void OnButtonEdgeCases();

};
