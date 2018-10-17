#include "TestGenerators.h"

#include <random>

namespace TestGenerators {

void DualGridTest(uint64_t seed, uint32_t grid_size, double grid_angle, DualGridType type, bool holes, Polygon *results) {
	assert(grid_size != 0);

	std::mt19937_64 rng(seed * SEED_MULT + SEED_ADD);
	double grid_step = 700.0 / double(grid_size), grid_rot = grid_angle * M_PI / 180.0;

	// select distributions
	std::uniform_int_distribution<uint32_t> dist1;
	std::uniform_real_distribution<double> dist2(grid_step * 0.25, grid_step * 0.49);
	switch(type) {
		case DUALGRID_DEFAULT: {
			dist1 = std::uniform_int_distribution<uint32_t>(3, 17);
			dist2 = std::uniform_real_distribution<double>(grid_step * 0.25, grid_step * 0.49);
			break;
		}
		case DUALGRID_STARS: {
			dist1 = std::uniform_int_distribution<uint32_t>(50, 150);
			dist2 = std::uniform_real_distribution<double>(grid_step * 0.10, grid_step * 0.49);
			break;
		}
		case DUALGRID_CIRCLES: {
			dist1 = std::uniform_int_distribution<uint32_t>(50, 150);
			dist2 = std::uniform_real_distribution<double>(grid_step * 0.39, grid_step * 0.41);
			break;
		}
	}

	// generate polygons
	for(uint32_t k = 0; k < 2; ++k) {
		results[k].Clear();
		double rot = (k == 0)? grid_rot : -grid_rot;
		if(holes) {
			double r = double(grid_size) * grid_step / 2.0;
			results[k].AddVertex(Vertex(-r * cos(rot) - r * sin(rot), -r * cos(rot) + r * sin(rot)));
			results[k].AddVertex(Vertex( r * cos(rot) - r * sin(rot), -r * cos(rot) - r * sin(rot)));
			results[k].AddVertex(Vertex( r * cos(rot) + r * sin(rot),  r * cos(rot) - r * sin(rot)));
			results[k].AddVertex(Vertex(-r * cos(rot) + r * sin(rot),  r * cos(rot) + r * sin(rot)));
			results[k].AddLoopEnd(1);
		}
		for(uint32_t gx = 0; gx < grid_size; ++gx) {
			uint32_t gx2 = (13229 * gx) % grid_size;
			for(uint32_t gy = 0; gy < grid_size; ++gy) {
				uint32_t gy2 = (15937 * gy) % grid_size;
				uint32_t n = dist1(rng);
				double x1 = double(2 * int32_t(gx2) - int32_t(grid_size) + 1) / 2.0 * grid_step;
				double y1 = double(2 * int32_t(gy2) - int32_t(grid_size) + 1) / 2.0 * grid_step;
				double x2 = x1 * cos(rot) + y1 * sin(rot);
				double y2 = y1 * cos(rot) - x1 * sin(rot);
				for(uint32_t j = 0; j < n; ++j) {
					double angle = double(j) / double(n) * 2.0 * M_PI;
					double radius = dist2(rng);
					results[k].AddVertex(Vertex(
						x2 + cos(angle) * radius,
						y2 + sin(angle) * radius
					));
				}
				results[k].AddLoopEnd((holes)? -1 : 1);
			}
		}
	}

}

Polygon EdgeCaseTest(uint64_t seed, uint32_t num_lines, uint32_t num_polygons) {
	assert(num_lines != 0);
	assert(num_polygons != 0);

	struct Line {
		Vertex m_v1, m_v2;
	};

	// initialize rng
	std::mt19937_64 rng(seed * SEED_MULT + SEED_ADD);
	std::uniform_real_distribution<double> dist_coord(-440.0, 440.0);
	std::uniform_real_distribution<double> dist_t(0.0, 1.0);
	std::uniform_real_distribution<double> dist_eps(-1e-13, 1e-13);
	std::uniform_int_distribution<uint32_t> dist_line(0, num_lines - 1);

	auto RandomLine = [&]() {
		if(rng() & 1) {
			double y = dist_coord(rng);
			return Line{
				{dist_coord(rng), y + dist_eps(rng)},
				{dist_coord(rng), y + dist_eps(rng)},
			};
		} else {
			return Line{
				{dist_coord(rng), dist_coord(rng)},
				{dist_coord(rng), dist_coord(rng)},
			};
		}
	};

	// generate random lines
	std::vector<Line> lines(num_lines);
	for(uint32_t i = 0; i < num_lines; ++i) {
		lines[i] = RandomLine();
	}

	// generate polygons
	Polygon result;
	for(uint32_t i = 0; i < num_polygons; ++i) {

		// generate one polygon
		for(uint32_t j = 0; j < num_lines; ++j) {
			auto &line = lines[dist_line(rng)];
			uint32_t flip = rng() & 1;
			for(uint32_t k = 0; k < 2; ++k) {
				double t = (double(k ^ flip) + dist_t(rng)) / 2.0;
				result.AddVertex(Vertex(
					line.m_v1.x() + (line.m_v2.x() - line.m_v1.x()) * t + dist_eps(rng),
					line.m_v1.y() + (line.m_v2.y() - line.m_v1.y()) * t + dist_eps(rng)//*3e13
				));
			}
		}
		result.AddLoopEnd(1);

		// replace a random line
		lines[dist_line(rng)] = RandomLine();

	}

	return result;
}

}
