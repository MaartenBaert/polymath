#include "TestGenerators.h"

#include <random>

// TODO: remove
#include <iostream>

namespace TestGenerators {

void DualGrid(uint64_t seed, DualGridType type, uint32_t grid_size, double grid_angle, bool holes, Polygon results[]) {
	assert(grid_size != 0);

	std::mt19937_64 rng(seed * SEED_MULT + SEED_ADD);
	double grid_step = 1.4 / double(grid_size), grid_rot = M_PI / 180.0 * grid_angle;

	// select distributions
	std::uniform_int_distribution<uint32_t> dist1;
	std::uniform_real_distribution<double> dist2(0.25 * grid_step, 0.49 * grid_step);
	switch(type) {
		case DUALGRID_DEFAULT: {
			dist1 = std::uniform_int_distribution<uint32_t>(3, 17);
			dist2 = std::uniform_real_distribution<double>(0.25 * grid_step, 0.49 * grid_step);
			break;
		}
		case DUALGRID_STARS: {
			dist1 = std::uniform_int_distribution<uint32_t>(50, 150);
			dist2 = std::uniform_real_distribution<double>(0.10 * grid_step, 0.49 * grid_step);
			break;
		}
		case DUALGRID_CIRCLES: {
			dist1 = std::uniform_int_distribution<uint32_t>(50, 150);
			dist2 = std::uniform_real_distribution<double>(0.39 * grid_step, 0.41 * grid_step);
			break;
		}
	}

	// generate polygons
	for(uint32_t k = 0; k < 2; ++k) {
		results[k].Clear();
		double rot = (k == 0)? grid_rot : -grid_rot;
		if(holes) {
			double r = double(grid_size) * grid_step / 2.0;
			results[k].AddVertex(Vertex(-r * cos(rot) + r * sin(rot), -r * cos(rot) - r * sin(rot)));
			results[k].AddVertex(Vertex( r * cos(rot) + r * sin(rot), -r * cos(rot) + r * sin(rot)));
			results[k].AddVertex(Vertex( r * cos(rot) - r * sin(rot),  r * cos(rot) + r * sin(rot)));
			results[k].AddVertex(Vertex(-r * cos(rot) - r * sin(rot),  r * cos(rot) - r * sin(rot)));
			results[k].AddLoopEnd(1);
		}
		for(uint32_t gx = 0; gx < grid_size; ++gx) {
			uint32_t gx2 = (13229 * gx) % grid_size;
			for(uint32_t gy = 0; gy < grid_size; ++gy) {
				uint32_t gy2 = (15937 * gy) % grid_size;
				uint32_t n = dist1(rng);
				double x1 = 0.5 * grid_step * double(2 * int32_t(gx2) - int32_t(grid_size) + 1);
				double y1 = 0.5 * grid_step * double(2 * int32_t(gy2) - int32_t(grid_size) + 1);
				double x2 = x1 * cos(rot) - y1 * sin(rot);
				double y2 = y1 * cos(rot) + x1 * sin(rot);
				for(uint32_t j = 0; j < n; ++j) {
					double angle = 2.0 * M_PI * double(j) / double(n);
					double radius = dist2(rng);
					results[k].AddVertex(Vertex(
						x2 + radius * cos(rot + angle),
						y2 + radius * sin(rot + angle)
					));
				}
				results[k].AddLoopEnd((holes)? -1 : 1);
			}
		}
	}

}

Polygon Orthogonal(uint64_t seed, uint32_t num_lines, uint32_t num_polygons) {
	assert(num_lines != 0);
	assert(num_polygons != 0);

	// initialize rng
	std::mt19937_64 rng(seed * SEED_MULT + SEED_ADD);
	std::uniform_real_distribution<double> dist_coord(-0.95, 0.95);
	std::uniform_real_distribution<double> dist_t(0.0, 1.0);
	std::uniform_int_distribution<uint32_t> dist_line(0, num_lines - 1);
	std::uniform_int_distribution<uint32_t> dist_num(1, 10);

	// generate random lines
	std::vector<double> lines_x(num_lines), lines_y(num_lines);
	for(uint32_t i = 0; i < num_lines; ++i) {
		lines_x[i] = dist_coord(rng);
		lines_y[i] = dist_coord(rng);
	}

	// generate polygons
	Polygon result;
	for(uint32_t i = 0; i < num_polygons; ++i) {

		// generate one polygon
		double start_y = lines_y[dist_line(rng)], curr_x, curr_y = start_y;
		uint32_t num = dist_num(rng);
		for(uint32_t j = 0; j < num; ++j) {
			curr_x = lines_x[dist_line(rng)];
			result.AddVertex(Vertex(curr_x, curr_y));
			curr_y = lines_y[dist_line(rng)];
			result.AddVertex(Vertex(curr_x, curr_y));
		}
		curr_x = lines_x[dist_line(rng)];
		result.AddVertex(Vertex(curr_x, curr_y));
		curr_y = start_y;
		result.AddVertex(Vertex(curr_x, curr_y));
		result.AddLoopEnd(1);

		// replace a random line
		lines_x[dist_line(rng)] = dist_coord(rng);
		lines_y[dist_line(rng)] = dist_coord(rng);

	}

	return result;
}

Polygon EdgeCases(uint64_t seed, uint32_t num_lines, uint32_t num_polygons, double eps) {
	assert(num_lines != 0);
	assert(num_polygons != 0);

	struct Line {
		Vertex m_v1, m_v2;
	};

	// initialize rng
	std::mt19937_64 rng(seed * SEED_MULT + SEED_ADD);
	std::uniform_real_distribution<double> dist_coord(-0.95, 0.95);
	std::uniform_real_distribution<double> dist_t(0.0, 1.0);
	std::normal_distribution<double> dist_eps(0.0, eps);
	std::uniform_int_distribution<uint32_t> dist_line(0, num_lines - 1);

	auto RandomLine = [&]() {
		Line line;
		switch(rng() & 3) {
			case 0: {
				double x = dist_coord(rng);
				line.m_v1 = Vertex(x + dist_eps(rng), dist_coord(rng));
				line.m_v2 = Vertex(x + dist_eps(rng), dist_coord(rng));
				break;
			}
			case 1: {
				double y = dist_coord(rng);
				line.m_v1 = Vertex(dist_coord(rng), y + dist_eps(rng));
				line.m_v2 = Vertex(dist_coord(rng), y + dist_eps(rng));
				break;
			}
			default: {
				line.m_v1 = Vertex(dist_coord(rng), dist_coord(rng));
				line.m_v2 = Vertex(dist_coord(rng), dist_coord(rng));
				break;
			}
		}
		return line;
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
					line.m_v1.x + (line.m_v2.x - line.m_v1.x) * t + dist_eps(rng),
					line.m_v1.y + (line.m_v2.y - line.m_v1.y) * t + dist_eps(rng)
				));
			}
		}
		result.AddLoopEnd(1);

		// replace a random line
		lines[dist_line(rng)] = RandomLine();

	}

	return result;
}

Polygon Star(uint32_t num_points) {
	assert(num_points != 0);

	// generate polygon
	Polygon result;
	uint32_t num = 2 * num_points + 1;
	for(uint32_t i = 0; i < num; ++i) {
		double angle = M_PI * double(i) * double(num - 1) / double(num);
		double radius = 0.95;
		result.AddVertex(Vertex(
			cos(angle) * radius,
			sin(angle) * radius
		));
	}
	result.AddLoopEnd(1);

	return result;
}

}
