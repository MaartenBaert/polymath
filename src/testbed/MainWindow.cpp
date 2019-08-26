#include "MainWindow.h"

#include "polymath/PolyMath.h"
#include "testgenerators/TestGenerators.h"
#include "testgenerators/TypeConverter.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>

template<typename F>
struct Conversion {

	typedef TestGenerators::Vertex Vertex;
	typedef TestGenerators::Polygon Polygon;

	typedef PolyMath::Vertex<F> Vertex2;
	typedef PolyMath::Polygon<F> Polygon2;
	typedef PolyMath::Visualization<F> Visualization2;

	static void Process(const Polygon &input, Visualizer *visualizer) {

		Polygon2 poly = TestGenerators::TypeConverter<F>::ConvertPolygonToType(input);

		PolyMath::SweepEngine<F, PolyMath::WindingEngine_Positive<typename Polygon2::WindingWeightType>> engine(poly);
		engine.Process();

		std::unique_ptr<VisualizationWrapper<F>> wrapper(new VisualizationWrapper<F>());
		wrapper->SetPolygonInput(poly);
		wrapper->SetPolygonOutput(engine.Result());
		visualizer->SetWrapper(std::move(wrapper));

	}

	static void Visualize(const Polygon &input, Visualizer *visualizer) {

		Polygon2 poly = TestGenerators::TypeConverter<F>::ConvertPolygonToType(input);

		PolyMath::SweepEngine<F, PolyMath::WindingEngine_Positive<typename Polygon2::WindingWeightType>> engine(poly);
		engine.Process([&](){
			std::unique_ptr<VisualizationWrapper<F>> wrapper(new VisualizationWrapper<F>());
			wrapper->SetPolygonInput(poly);
			wrapper->SetVisualization(engine.Visualize());
			visualizer->SetWrapper(std::move(wrapper));
			visualizer->update();
			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			//QMessageBox::information(nullptr, "Visualization", "...");
		});

		std::unique_ptr<VisualizationWrapper<F>> wrapper(new VisualizationWrapper<F>());
		wrapper->SetPolygonInput(poly);
		wrapper->SetPolygonOutput(engine.Result());
		visualizer->SetWrapper(std::move(wrapper));

	}

	static void EdgeCases(size_t num_tests, size_t num_probes) {

		double eps = TestGenerators::TypeConverter<F>::Epsilon();

		std::mt19937_64 rng(UINT64_C(0xc0d9e78ec6150647));
		std::uniform_real_distribution<double> dist_t(0.0, 1.0);
		std::uniform_real_distribution<double> dist_eps(-eps, eps);

		auto flags = std::cerr.flags();
		std::cerr << std::scientific;

		double probe_max_dist = 0.0;
		size_t probe_errors = 0;
		for(size_t seed = 0; seed < num_tests; ++seed) {

			// generate polygon
			Polygon poly = TestGenerators::EdgeCases(seed, 5, 10, 2.0 * eps);

			// process
			Polygon2 poly_conv = TestGenerators::TypeConverter<F>::ConvertPolygonToType(poly);
			PolyMath::SweepEngine<F, PolyMath::WindingEngine_EvenOdd<typename Polygon2::WindingWeightType>> engine(poly_conv);
			engine.Process();
			Polygon2 result_conv = engine.Result();
			Polygon result = TestGenerators::TypeConverter<F>::ConvertPolygonFromType(result_conv);

			// probe
			for(size_t j = 0; j < num_probes; ++j) {
				bool swap = false; //rng() & 1;
				Polygon &poly1 = (swap)? result : poly;
				Polygon &poly2 = (swap)? poly : result;
				size_t loop = std::uniform_int_distribution<size_t>(0, poly1.loops.size() - 1)(rng);
				Vertex *vertices = poly1.GetLoopVertices(loop);
				size_t vertex_count = poly1.GetLoopVertexCount(loop);
				size_t k = std::uniform_int_distribution<size_t>(0, vertex_count - 1)(rng);
				Vertex v1 = vertices[k], v2 = vertices[(k + 1) % vertex_count];
				double t = dist_t(rng);
				Vertex point = {
					v1.x + (v2.x - v1.x) * t + dist_eps(rng),
					v1.y + (v2.y - v1.y) * t + dist_eps(rng),
				};
				int64_t w1 = PolyMath::PolygonPointWindingNumber(poly1, point);
				int64_t w2 = PolyMath::PolygonPointWindingNumber(poly2, point);
				if((w1 & 1) != (w2 & 1)) {
					double dist = PolyMath::PolygonPointEdgeDistance(poly2, point);
					if(dist > 3.0 * eps) {
						std::cerr << "seed=" << seed << " dist=" << (dist / eps) << " point=" << point << std::endl;
					}
					probe_max_dist = std::max(probe_max_dist, dist);
					++probe_errors;
				}
			}

		}

		std::cerr << "Probe eps: " << eps << std::endl;
		std::cerr << "Probe max distance: " << (probe_max_dist / eps) << " eps" << std::endl;
		std::cerr << "Probe errors: " << probe_errors << " / " << (num_tests * num_probes) << std::endl;
		std::cerr.flags(flags);

	}

};

QStackedLayoutFixed::~QStackedLayoutFixed() {
	// nothing
}

Qt::Orientations QStackedLayoutFixed::expandingDirections() const {
	return 0;
}

MainWindow::MainWindow() {

	setWindowTitle("PolyMath Testbed");

	QWidget *centralwidget = new QWidget(this);
	setCentralWidget(centralwidget);

	QGroupBox *groupbox_settings = new QGroupBox("Settings", centralwidget);
	{
		m_settings_seed_spinbox = new QSpinBox(groupbox_settings);
		m_settings_seed_spinbox->setRange(0, 999999);
		m_settings_seed_spinbox->setValue(0);
		m_settings_type_combobox = new QComboBox(groupbox_settings);
		m_settings_type_combobox->addItems({"Int 8-bit", "Int 16-bit", "Int 32-bit", "Int 64-bit", "Float 32-bit", "Float 64-bit"});
		m_settings_type_combobox->setCurrentIndex(VALUETYPE_F32);
		m_settings_fusion_checkbox = new QCheckBox("Fusion", groupbox_settings);

		connect(m_settings_seed_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
		connect(m_settings_type_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTestChanged()));
		connect(m_settings_fusion_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnTestChanged()));

		QFormLayout *layout = new QFormLayout(groupbox_settings);
		layout->addRow("Type", m_settings_type_combobox);
		layout->addRow("Seed", m_settings_seed_spinbox);
		layout->addRow(m_settings_fusion_checkbox);
	}
	QGroupBox *groupbox_test = new QGroupBox("Test", centralwidget);
	{
		m_test_type_combobox = new QComboBox(groupbox_test);
		m_test_type_combobox->addItems({"Dual Grid", "Orthogonal", "Edge Cases", "Star"});
		m_test_type_combobox->setCurrentIndex(TESTTYPE_DUALGRID);
		
		QWidget *page_dualgrid = new QWidget(groupbox_test);
		{
			m_test_dualgrid_type_combobox = new QComboBox(page_dualgrid);
			m_test_dualgrid_type_combobox->addItems({"Default", "Stars", "Circles"});
			m_test_dualgrid_type_combobox->setCurrentIndex(TestGenerators::DUALGRID_DEFAULT);
			m_test_dualgrid_size_spinbox = new QSpinBox(page_dualgrid);
			m_test_dualgrid_size_spinbox->setRange(1, 100);
			m_test_dualgrid_size_spinbox->setValue(3);
			m_test_dualgrid_angle_spinbox = new QDoubleSpinBox(page_dualgrid);
			m_test_dualgrid_angle_spinbox->setRange(0.0, 90.0);
			m_test_dualgrid_angle_spinbox->setDecimals(1);
			m_test_dualgrid_angle_spinbox->setSingleStep(0.1);
			m_test_dualgrid_angle_spinbox->setValue(20.0);
			m_test_dualgrid_holes_checkbox = new QCheckBox("Holes", page_dualgrid);

			connect(m_test_dualgrid_type_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTestChanged()));
			connect(m_test_dualgrid_size_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
			connect(m_test_dualgrid_angle_spinbox, SIGNAL(valueChanged(double)), this, SLOT(OnTestChanged()));
			connect(m_test_dualgrid_holes_checkbox, SIGNAL(stateChanged(int)), this, SLOT(OnTestChanged()));

			QFormLayout *layout = new QFormLayout(page_dualgrid);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addRow("Type", m_test_dualgrid_type_combobox);
			layout->addRow("Size", m_test_dualgrid_size_spinbox);
			layout->addRow("Angle", m_test_dualgrid_angle_spinbox);
			layout->addRow(m_test_dualgrid_holes_checkbox);
		}
		QWidget *page_orthogonal = new QWidget(groupbox_test);
		{
			m_test_orthogonal_lines_spinbox = new QSpinBox(page_orthogonal);
			m_test_orthogonal_lines_spinbox->setRange(1, 100);
			m_test_orthogonal_lines_spinbox->setValue(20);
			m_test_orthogonal_polygons_spinbox = new QSpinBox(page_orthogonal);
			m_test_orthogonal_polygons_spinbox->setRange(1, 100);
			m_test_orthogonal_polygons_spinbox->setValue(10);

			connect(m_test_orthogonal_lines_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
			connect(m_test_orthogonal_polygons_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));

			QFormLayout *layout = new QFormLayout(page_orthogonal);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addRow("Lines", m_test_orthogonal_lines_spinbox);
			layout->addRow("Polygons", m_test_orthogonal_polygons_spinbox);
		}
		QWidget *page_edgecases = new QWidget(groupbox_test);
		{
			m_test_edgecases_lines_spinbox = new QSpinBox(page_edgecases);
			m_test_edgecases_lines_spinbox->setRange(1, 100);
			m_test_edgecases_lines_spinbox->setValue(5);
			m_test_edgecases_polygons_spinbox = new QSpinBox(page_edgecases);
			m_test_edgecases_polygons_spinbox->setRange(1, 100);
			m_test_edgecases_polygons_spinbox->setValue(10);
			m_test_edgecases_epsilon_spinbox = new QDoubleSpinBox(page_edgecases);
			m_test_edgecases_epsilon_spinbox->setRange(0.0, 10.0);
			m_test_edgecases_epsilon_spinbox->setDecimals(1);
			m_test_edgecases_epsilon_spinbox->setSingleStep(0.1);
			m_test_edgecases_epsilon_spinbox->setValue(2.0);

			connect(m_test_edgecases_lines_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
			connect(m_test_edgecases_polygons_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
			connect(m_test_edgecases_epsilon_spinbox, SIGNAL(valueChanged(double)), this, SLOT(OnTestChanged()));

			QFormLayout *layout = new QFormLayout(page_edgecases);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addRow("Lines", m_test_edgecases_lines_spinbox);
			layout->addRow("Polygons", m_test_edgecases_polygons_spinbox);
			layout->addRow("Epsilon", m_test_edgecases_epsilon_spinbox);
		}
		QWidget *page_star = new QWidget(groupbox_test);
		{
			m_test_star_points_spinbox = new QSpinBox(page_star);
			m_test_star_points_spinbox->setRange(1, 1000);
			m_test_star_points_spinbox->setValue(10);

			connect(m_test_star_points_spinbox, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));

			QFormLayout *layout = new QFormLayout(page_star);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addRow("Points", m_test_star_points_spinbox);
		}

		connect(m_test_type_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTestChanged()));

		QFormLayout *layout = new QFormLayout(groupbox_test);
		layout->addRow("Type", m_test_type_combobox);
		{
			m_test_stackedlayout = new QStackedLayoutFixed();
			layout->addRow(m_test_stackedlayout);
			m_test_stackedlayout->addWidget(page_dualgrid);
			m_test_stackedlayout->addWidget(page_orthogonal);
			m_test_stackedlayout->addWidget(page_edgecases);
			m_test_stackedlayout->addWidget(page_star);
		}
	}
	QGroupBox *groupbox_actions = new QGroupBox("Actions", centralwidget);
	{
		QPushButton *button_visualize = new QPushButton("Visualize", groupbox_actions);
		QPushButton *button_edgecases = new QPushButton("Edge Cases", groupbox_actions);

		connect(button_visualize, SIGNAL(clicked()), this, SLOT(OnButtonVisualize()));
		connect(button_edgecases, SIGNAL(clicked()), this, SLOT(OnButtonEdgeCases()));

		QFormLayout *layout = new QFormLayout(groupbox_actions);
		layout->addRow(button_visualize);
		layout->addRow(button_edgecases);
	}
	m_visualizer = new Visualizer(centralwidget);

	QHBoxLayout *layout = new QHBoxLayout(centralwidget);
	{
		QVBoxLayout *layout2 = new QVBoxLayout();
		layout->addLayout(layout2);
		layout2->addWidget(groupbox_settings);
		layout2->addWidget(groupbox_test);
		layout2->addWidget(groupbox_actions);
		layout2->addStretch();
	}
	layout->addWidget(m_visualizer);

	OnTestChanged();

	show();
}

MainWindow::~MainWindow() {
	// nothing
}

double MainWindow::GetEpsilon(MainWindow::ValueType type) {
	switch(type) {
		case VALUETYPE_I8 : return TestGenerators::TypeConverter<int8_t >::Epsilon();
		case VALUETYPE_I16: return TestGenerators::TypeConverter<int16_t>::Epsilon();
		case VALUETYPE_I32: return TestGenerators::TypeConverter<int32_t>::Epsilon();
		case VALUETYPE_I64: return TestGenerators::TypeConverter<int64_t>::Epsilon();
		case VALUETYPE_F32: return TestGenerators::TypeConverter<float  >::Epsilon();
		case VALUETYPE_F64: return TestGenerators::TypeConverter<double >::Epsilon();
	}
	assert(false);
	return 0.0;
}

void MainWindow::OnTestChanged() {

	uint64_t settings_seed = uint64_t(m_settings_seed_spinbox->value());
	ValueType settings_type = ValueType(m_settings_type_combobox->currentIndex());
	bool settings_fusion = m_settings_fusion_checkbox->isChecked();

	TestType testtype = TestType(m_test_type_combobox->currentIndex());
	m_test_stackedlayout->setCurrentIndex(testtype);

	switch(testtype) {
		case TESTTYPE_DUALGRID: {
			TestGenerators::DualGridType dualgrid_type = TestGenerators::DualGridType(m_test_dualgrid_type_combobox->currentIndex());
			uint32_t dualgrid_size = uint32_t(m_test_dualgrid_size_spinbox->value());
			double dualgrid_angle = m_test_dualgrid_angle_spinbox->value();
			bool dualgrid_holes = m_test_dualgrid_holes_checkbox->isChecked();
			Polygon inputs[2];
			TestGenerators::DualGrid(settings_seed, dualgrid_type, dualgrid_size, dualgrid_angle, dualgrid_holes, inputs);
			m_polygon = inputs[0];
			m_polygon += inputs[1];
			break;
		}
		case TESTTYPE_ORTHOGONAL: {
			uint32_t orthogonal_lines = uint32_t(m_test_orthogonal_lines_spinbox->value());
			uint32_t orthogonal_polygons = uint32_t(m_test_orthogonal_polygons_spinbox->value());
			m_polygon = TestGenerators::Orthogonal(settings_seed, orthogonal_lines, orthogonal_polygons);
			break;
		}
		case TESTTYPE_EDGECASES: {
			uint32_t edgecases_lines = uint32_t(m_test_edgecases_lines_spinbox->value());
			uint32_t edgecases_polygons = uint32_t(m_test_edgecases_polygons_spinbox->value());
			double edgecases_epsilon = m_test_edgecases_epsilon_spinbox->value();
			m_polygon = TestGenerators::EdgeCases(settings_seed, edgecases_lines, edgecases_polygons, edgecases_epsilon * GetEpsilon(settings_type));
			break;
		}
		case TESTTYPE_STAR: {
			uint32_t star_points = uint32_t(m_test_star_points_spinbox->value());
			m_polygon = TestGenerators::Star(star_points);
			break;
		}
	}

	switch(settings_type) {
		case VALUETYPE_I8 : Conversion<int8_t >::Process(m_polygon, m_visualizer); break;
		case VALUETYPE_I16: Conversion<int16_t>::Process(m_polygon, m_visualizer); break;
		case VALUETYPE_I32: Conversion<int32_t>::Process(m_polygon, m_visualizer); break;
		case VALUETYPE_I64: Conversion<int64_t>::Process(m_polygon, m_visualizer); break;
		case VALUETYPE_F32: Conversion<float  >::Process(m_polygon, m_visualizer); break;
		case VALUETYPE_F64: Conversion<double >::Process(m_polygon, m_visualizer); break;
	}

}

void MainWindow::OnButtonVisualize() {

	ValueType type = ValueType(m_settings_type_combobox->currentIndex());
	switch(type) {
		case VALUETYPE_I8 : Conversion<int8_t >::Visualize(m_polygon, m_visualizer); break;
		case VALUETYPE_I16: Conversion<int16_t>::Visualize(m_polygon, m_visualizer); break;
		case VALUETYPE_I32: Conversion<int32_t>::Visualize(m_polygon, m_visualizer); break;
		case VALUETYPE_I64: Conversion<int64_t>::Visualize(m_polygon, m_visualizer); break;
		case VALUETYPE_F32: Conversion<float  >::Visualize(m_polygon, m_visualizer); break;
		case VALUETYPE_F64: Conversion<double >::Visualize(m_polygon, m_visualizer); break;
	}

}

void MainWindow::OnButtonEdgeCases() {

	size_t num_tests = 1000, num_probes = 100;

	ValueType type = ValueType(m_settings_type_combobox->currentIndex());
	switch(type) {
		case VALUETYPE_I8 : Conversion<int8_t >::EdgeCases(num_tests, num_probes); break;
		case VALUETYPE_I16: Conversion<int16_t>::EdgeCases(num_tests, num_probes); break;
		case VALUETYPE_I32: Conversion<int32_t>::EdgeCases(num_tests, num_probes); break;
		case VALUETYPE_I64: Conversion<int64_t>::EdgeCases(num_tests, num_probes); break;
		case VALUETYPE_F32: Conversion<float  >::EdgeCases(num_tests, num_probes); break;
		case VALUETYPE_F64: Conversion<double >::EdgeCases(num_tests, num_probes); break;
	}

}
