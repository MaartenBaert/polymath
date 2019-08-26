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

private:
	enum ValueType {
		VALUETYPE_I8,
		VALUETYPE_I16,
		VALUETYPE_I32,
		VALUETYPE_I64,
		VALUETYPE_F32,
		VALUETYPE_F64,
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
	QCheckBox *m_settings_fusion_checkbox;

	// test
	QComboBox *m_test_type_combobox;
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

	Visualizer *m_visualizer;

public:
	MainWindow();
	~MainWindow();

private:
	static double GetEpsilon(ValueType type);

public slots:
	void OnTestChanged();
	void OnButtonVisualize();
	void OnButtonEdgeCases();

};
