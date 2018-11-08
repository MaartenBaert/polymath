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

template<typename F>
struct Conversion {

	typedef TestGenerators::Vertex Vertex;
	typedef TestGenerators::Polygon Polygon;

	typedef PolyMath::Vertex<F> Vertex2;
	typedef PolyMath::Polygon<Vertex2> Polygon2;
	typedef PolyMath::Visualization<Vertex2> Visualization2;

	static Polygon2 PolyConvert(const Polygon &input) {
		Polygon2 output;
		output.ReserveLoops(input.GetLoopCount());
		output.ReserveVertices(output.GetVertexCount());
		for(size_t i = 0; i < input.GetLoopCount(); i++) {
			const Vertex *vertices = input.GetLoopVertices(i);
			size_t n = input.GetLoopVertexCount(i);
			for(size_t j = 0; j < n; ++j) {
				const Vertex &v = vertices[j];
				output.AddVertex(Vertex2(F(v.x()), F(v.y())));
			}
			output.AddLoopEnd(input.GetLoopWindingWeight(i));
		}
		return output;
	}

	static void Process(const Polygon &input, Visualizer *visualizer) {

		Polygon2 poly = PolyConvert(input);

		PolyMath::SweepEngine<Vertex2, PolyMath::NumericalEngine<F>, PolyMath::WindingEngine_Xor> engine(poly);
		engine.Process(PolyMath::DummyVisualizationCallback);

		std::unique_ptr<VisualizationWrapper<F>> wrapper(new VisualizationWrapper<F>());
		wrapper->SetPolygonInput(poly);
		wrapper->SetPolygonOutput(engine.Result());
		visualizer->SetWrapper(std::move(wrapper));

	}

	static void Visualize(const Polygon &input, Visualizer *visualizer) {

		Polygon2 poly = PolyConvert(input);

		PolyMath::SweepEngine<Vertex2, PolyMath::NumericalEngine<F>, PolyMath::WindingEngine_Xor> engine(poly);
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

};

MainWindow::MainWindow() {

	setWindowTitle("PolyMath Testbed");

	QWidget *centralwidget = new QWidget(this);
	setCentralWidget(centralwidget);

	QGroupBox *groupbox_basic = new QGroupBox("Basic", centralwidget);
	{
		m_combobox_type = new QComboBox(groupbox_basic);
		m_combobox_type->addItem("F32");
		m_combobox_type->addItem("F64");
		m_spinbox_seed = new QSpinBox(groupbox_basic);
		m_spinbox_seed->setRange(0, 999999);
		m_spinbox_seed->setValue(747);

		connect(m_combobox_type, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTestChanged()));
		connect(m_spinbox_seed, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));

		QFormLayout *layout = new QFormLayout(groupbox_basic);
		layout->addRow("Type", m_combobox_type);
		layout->addRow("Seed", m_spinbox_seed);
	}
	QGroupBox *groupbox_test = new QGroupBox("Test", centralwidget);
	{
		m_spinbox_size = new QSpinBox(groupbox_test);
		m_spinbox_size->setRange(1, 100);
		m_spinbox_size->setValue(3);
		m_spinbox_angle = new QDoubleSpinBox(groupbox_test);
		m_spinbox_angle->setRange(0.0, 90.0);
		m_spinbox_angle->setDecimals(1);
		m_spinbox_angle->setSingleStep(0.1);
		m_spinbox_angle->setValue(20.0);

		connect(m_spinbox_size, SIGNAL(valueChanged(int)), this, SLOT(OnTestChanged()));
		connect(m_spinbox_angle, SIGNAL(valueChanged(double)), this, SLOT(OnTestChanged()));

		QFormLayout *layout = new QFormLayout(groupbox_test);
		layout->addRow("Size", m_spinbox_size);
		layout->addRow("Angle", m_spinbox_angle);
	}
	QGroupBox *groupbox_actions = new QGroupBox("Actions", centralwidget);
	{
		QPushButton *button_visualize = new QPushButton("Visualize", groupbox_actions);
		QPushButton *button_benchmark = new QPushButton("Benchmark", groupbox_actions);
		QPushButton *button_edgecases = new QPushButton("Edge Cases", groupbox_actions);

		connect(button_visualize, SIGNAL(clicked()), this, SLOT(OnButtonVisualize()));
		connect(button_benchmark, SIGNAL(clicked()), this, SLOT(OnButtonBenchmark()));
		connect(button_edgecases, SIGNAL(clicked()), this, SLOT(OnButtonEdgeCases()));

		QFormLayout *layout = new QFormLayout(groupbox_actions);
		layout->addRow(button_visualize);
		layout->addRow(button_benchmark);
		layout->addRow(button_edgecases);
	}
	m_visualizer = new Visualizer(centralwidget);

	QHBoxLayout *layout = new QHBoxLayout(centralwidget);
	{
		QVBoxLayout *layout2 = new QVBoxLayout();
		layout->addLayout(layout2);
		layout2->addWidget(groupbox_basic);
		layout2->addWidget(groupbox_test);
		layout2->addWidget(groupbox_actions);
		layout2->addStretch();
	}
	layout->addWidget(m_visualizer);

	OnTestChanged();

	show();
}

MainWindow::~MainWindow() {

}

void MainWindow::OnTestChanged() {

	uint64_t seed = uint64_t(m_spinbox_seed->value());
	uint32_t grid_size = uint32_t(m_spinbox_size->value());
	double grid_angle = m_spinbox_angle->value();

	Polygon inputs[2];
	TestGenerators::DualGridTest(seed, grid_size, grid_angle, TestGenerators::DUALGRID_DEFAULT, false, inputs);
	m_polygon = inputs[0];
	m_polygon += inputs[1];

	//m_polygon = TestGenerators::EdgeCaseTest(seed, grid_size, 2 * grid_size);

	Type type = Type(m_combobox_type->currentIndex());
	switch(type) {
		case TYPE_F32: Conversion<float>::Process(m_polygon, m_visualizer); break;
		case TYPE_F64: Conversion<double>::Process(m_polygon, m_visualizer); break;
	}

}

void MainWindow::OnButtonVisualize() {

	Type type = Type(m_combobox_type->currentIndex());
	switch(type) {
		case TYPE_F32: Conversion<float>::Visualize(m_polygon, m_visualizer); break;
		case TYPE_F64: Conversion<double>::Visualize(m_polygon, m_visualizer); break;
	}

}

void MainWindow::OnButtonBenchmark() {

	struct Benchmark {
		std::string m_name;
		double (*m_func)(const Polygon&, const Polygon&, Polygon&, size_t);
		bool m_run;
	};

	std::vector<Benchmark> benchmarks = {
		{"PolyMath F32", PolyMathWrapper::BenchmarkUnion_F32, true},
		{"PolyMath F64", PolyMathWrapper::BenchmarkUnion_F64, true},
		{"Boost F32"   , BoostWrapper   ::BenchmarkUnion_F32, true},
		{"Boost F64"   , BoostWrapper   ::BenchmarkUnion_F64, true},
		{"Clipper"     , ClipperWrapper ::BenchmarkUnion    , true},
		{"Geos"        , GeosWrapper    ::BenchmarkUnion    , true},
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

		Polygon inputs[2];
		TestGenerators::DualGridTest(0, tests[tnum], 20.0, TestGenerators::DUALGRID_DEFAULT, false, inputs);

		std::cout << W << (inputs[0].GetVertexCount() + inputs[1].GetVertexCount());
		std::cout.flush();

		for(Benchmark &benchmark : benchmarks) {
			Polygon result;
			double time = 0.0;
			if(benchmark.m_run) {
				size_t loops = 1;
				for( ; ; ) {
					time = benchmark.m_func(inputs[0], inputs[1], result, loops); // benchmark.m_loops / polygon.GetVertexCount() + 1
					if(time * double(loops) > 1.0)
						break;
					loops = std::max(loops + 1, size_t(1.5 / time));
				}
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
	std::uniform_real_distribution<double> dist_eps(-1e-4, 1e-4);

	size_t num_tests = 10000, num_probes = 100;

	auto flags = std::cerr.flags();
	std::cerr << std::scientific;

	double probe_max_dist = 0.0;
	size_t probe_errors = 0;
	for(size_t i = 0; i < num_tests; ++i) {

		// generate polygon
		uint64_t seed = i;
		Polygon poly = TestGenerators::EdgeCaseTest(seed, 3, 6);

		// process
		PolyMath::SweepEngine<Vertex, PolyMath::NumericalEngine<typename Vertex::value_type>, PolyMath::WindingEngine_Xor> engine(poly);
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
