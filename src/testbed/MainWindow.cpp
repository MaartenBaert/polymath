#include "MainWindow.h"

#include "PolyMath.h"
#include "TestGenerators.h"

#include "PolyMathWrapper.h"
#include "BoostWrapper.h"
#include "ClipperWrapper.h"
#include "GeosWrapper.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>

MainWindow::MainWindow() {

	setWindowTitle("PolyMath Testbed");

	QWidget *centralwidget = new QWidget(this);
	setCentralWidget(centralwidget);

	QGroupBox *groupbox_basic = new QGroupBox("Basic", centralwidget);
	{
		m_spinbox_seed = new QSpinBox(groupbox_basic);
		m_spinbox_seed->setRange(0, 999999);
		m_spinbox_seed->setValue(747);
		m_spinbox_size = new QSpinBox(groupbox_basic);
		m_spinbox_size->setRange(1, 100);
		m_spinbox_size->setValue(2);
		m_spinbox_angle = new QDoubleSpinBox(groupbox_basic);
		m_spinbox_angle->setRange(0.0, 90.0);
		m_spinbox_angle->setDecimals(1);
		m_spinbox_angle->setSingleStep(0.1);
		m_spinbox_angle->setValue(20.0);
		QPushButton *button_simplify = new QPushButton("Simplify", groupbox_basic);
		QPushButton *button_benchmark = new QPushButton("Benchmark", groupbox_basic);
		QPushButton *button_edgecases = new QPushButton("Edge Cases", groupbox_basic);

		connect(m_spinbox_seed, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
		connect(m_spinbox_size, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
		connect(m_spinbox_angle, SIGNAL(valueChanged(double)), this, SLOT(OnTestChanged()));
		connect(button_simplify, SIGNAL(clicked()), this, SLOT(OnButtonSimplify()));
		connect(button_benchmark, SIGNAL(clicked()), this, SLOT(OnButtonBenchmark()));
		connect(button_edgecases, SIGNAL(clicked()), this, SLOT(OnButtonEdgeCases()));

		QFormLayout *layout = new QFormLayout(groupbox_basic);
		layout->addRow("Seed", m_spinbox_seed);
		layout->addRow("Size", m_spinbox_size);
		layout->addRow("Angle", m_spinbox_angle);
		layout->addRow(button_simplify);
		layout->addRow(button_benchmark);
		layout->addRow(button_edgecases);
	}
	m_visual = new Visualizer(centralwidget);

	QHBoxLayout *layout = new QHBoxLayout(centralwidget);
	{
		QVBoxLayout *layout2 = new QVBoxLayout();
		layout->addLayout(layout2);
		layout2->addWidget(groupbox_basic);
		layout2->addStretch();
	}
	layout->addWidget(m_visual);

	OnTestChanged();

	show();
}

MainWindow::~MainWindow() {

}

void MainWindow::MakeTest(uint64_t seed, uint32_t grid_size, double grid_angle) {

	/*Polygon inputs[2];
	TestGenerators::DualGridTest(seed, grid_size, grid_angle, TestGenerators::DUALGRID_DEFAULT, false, inputs);
	m_polygon = inputs[0];
	m_polygon += inputs[1];*/

	m_polygon = TestGenerators::EdgeCaseTest(seed, grid_size, 2 * grid_size);

	m_visual->SetPolygonInput(m_polygon);
	m_visual->SetPolygonOutput(Polygon());

}

void MainWindow::OnTestChanged() {
	MakeTest(uint64_t(m_spinbox_seed->value()), uint32_t(m_spinbox_size->value()), m_spinbox_angle->value());
	PolyMath::SweepEngine<Vertex, PolyMath::WindingEngine_Xor> engine(m_polygon);
	engine.Process(PolyMath::DummyVisualizationCallback);
	m_visual->SetPolygonOutput(engine.Result());
}

void MainWindow::OnButtonSimplify() {
	Polygon result;
	auto t1 = std::chrono::high_resolution_clock::now();
	{
		PolyMath::SweepEngine<Vertex, PolyMath::WindingEngine_Xor> engine(m_polygon);
		engine.Process([&](){
			m_visual->SetVisualization(engine.Visualize());
		});
		result = engine.Result();
	}
	auto t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;
	m_visual->RemoveVisualization();
	m_visual->SetPolygonOutput(result);
}

void MainWindow::OnButtonBenchmark() {

	struct Benchmark {
		std::string m_name;
		double (*m_func)(const Polygon&, const Polygon&, Polygon&, size_t);
		size_t m_loops;
		bool m_run;
	};

	std::vector<Benchmark> benchmarks = {
		{"PolyMath", PolyMathWrapper::BenchmarkUnion, 5000000, true},
		{"Boost"   , BoostWrapper   ::BenchmarkUnion,  200000, true},
		{"Clipper" , ClipperWrapper ::BenchmarkUnion,  100000, true},
		{"Geos"    , GeosWrapper    ::BenchmarkUnion,   10000, true},
	};

	auto W = std::setw(16);
	std::cout << W << "Test" << W << "Vertices";
	for(Benchmark &benchmark : benchmarks) {
		std::cout << W << benchmark.m_name;
	}
	std::cout << std::endl;

	std::vector<uint32_t> tests = {1, 2, 3, 5, 7, 10, 15, 22, 33, 47, 68, 100, 150, 220, 330, 470, 680, 1000, 1500, 2200};
	for(size_t tnum = 0; tnum < tests.size(); ++tnum) {

		std::cout << W << tests[tnum];
		std::cout.flush();

		Polygon inputs[2], polygon;
		TestGenerators::DualGridTest(0, tests[tnum], 20.0, TestGenerators::DUALGRID_DEFAULT, false, inputs);
		polygon = inputs[0];
		polygon += inputs[1];

		std::cout << W << polygon.GetVertexCount();
		std::cout.flush();

		for(Benchmark &benchmark : benchmarks) {
			Polygon result;
			double time = 0.0;
			if(benchmark.m_run) {
				time = benchmark.m_func(inputs[0], inputs[1], result, benchmark.m_loops / polygon.GetVertexCount() + 1);
				benchmark.m_run = (time < 10.0);
			}
			std::cout << W << time;
			std::cout.flush();
		}

		std::cout << std::endl;

	}
	std::cout << "Done." << std::endl;

}

void MainWindow::OnButtonEdgeCases() {

	std::mt19937_64 rng(UINT64_C(0xc0d9e78ec6150647));
	std::uniform_real_distribution<double> dist_t(0.0, 1.0);
	std::uniform_real_distribution<double> dist_eps(-1e-12, 1e-12);

	size_t num_tests = 10000, num_probes = 100;

	auto flags = std::cerr.flags();
	std::cerr << std::scientific;

	double probe_max_dist = 0.0;
	size_t probe_errors = 0;
	for(size_t i = 0; i < num_tests; ++i) {

		// generate polygon
		uint64_t seed = i;
		Polygon poly = TestGenerators::EdgeCaseTest(seed, 2, 4);

		// process
		PolyMath::SweepEngine<Vertex, PolyMath::WindingEngine_Xor> engine(poly);
		engine.Process(PolyMath::DummyVisualizationCallback);
		Polygon result = engine.Result();

		// probe
		for(size_t j = 0; j < num_probes; ++j) {
			bool swap = rng() & 1;
			Polygon &poly1 = (swap)? poly : result;
			Polygon &poly2 = (swap)? result : poly;
			size_t loop = std::uniform_int_distribution<size_t>(0, poly1.GetLoopCount() - 1)(rng);
			Vertex *vertices = poly1.GetLoopVertices(loop);
			size_t vertex_count = poly1.GetLoopVertexCount(loop);
			size_t k = std::uniform_int_distribution<size_t>(0, vertex_count - 1)(rng);
			Vertex v1 = vertices[k], v2 = vertices[(k + 1) % vertex_count];
			double t = dist_t(rng);
			Vertex point = {
				v1.x() + (v2.x() - v1.x()) * t + dist_eps(rng),
				v1.y() + (v2.y() - v1.y()) * t + dist_eps(rng),
			};
			int64_t w1 = PolyMath::PolygonPointWindingNumber(poly1, point);
			int64_t w2 = PolyMath::PolygonPointWindingNumber(poly2, point);
			if((w1 & 1) != (w2 & 1)) {
				double dist = PolyMath::PolygonPointEdgeDistance(poly2, point);
				if(dist > 3.0) {
					std::cerr << "seed=" << seed << " dist=" << dist << " point=" << point << std::endl;
				}
				probe_max_dist = std::max(probe_max_dist, dist);
				++probe_errors;
			}
		}

	}

	std::cerr << "Probe max distance: " << probe_max_dist << std::endl;
	std::cerr << "Probe errors: " << probe_errors << " / " << (num_tests * num_probes) << std::endl;
	std::cerr.flags(flags);

}
