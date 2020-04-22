#include "polymath/PolyMath.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>

#include <iostream>
#include <random>

typedef PolyMath::Vertex<float> Vertex;
typedef PolyMath::Polygon<float> Polygon;

struct Gear {
	float cx, cy, vx, vy, radius, angle, spin;
	uint32_t num;
};

SDL_Window *g_main_window = nullptr;
SDL_GLContext g_opengl_context = nullptr;

std::mt19937_64 g_rng;
std::vector<Gear> g_gears;

void AddRandomGear() {
	std::uniform_real_distribution<float> dist_cxy(0.0, 1.0);
	std::uniform_real_distribution<float> dist_vxy(-0.03, 0.03);
	std::uniform_real_distribution<float> dist_radius(0.02, 0.05);
	std::uniform_real_distribution<float> dist_angle(0.05, 0.10);
	std::uniform_real_distribution<float> dist_spin(-0.5, 0.5);
	std::uniform_int_distribution<uint32_t> dist_num(10, 20);
	g_gears.push_back(Gear{dist_cxy(g_rng), dist_cxy(g_rng), dist_vxy(g_rng), dist_vxy(g_rng), dist_radius(g_rng), dist_angle(g_rng), dist_spin(g_rng), dist_num(g_rng)});
}

void RemoveRandomGear() {
	if(g_gears.size() != 0) {
		std::uniform_int_distribution<size_t> dist_index(0, g_gears.size() - 1);
		g_gears.erase(g_gears.begin() + dist_index(g_rng));
	}
}

void GenerateGearPolygon(Polygon &poly, float cx, float cy, float radius, float angle, uint32_t num) {
	//(cx, cy) = (0.12 + 0.76 * rand(), 0.12 + 0.76 * rand())
	//radius = 0.06 + 0.06 * rand()
	//angle = 2 * pi * rand()
	//n = randint(10, 20)

	// outer loop
	for(size_t i = 0; i < 4 * num; ++i) {
		float r = (i % 4 < 2)? 0.9 * radius : 1.1 * radius;
		float theta = angle + float(2.0 * M_PI) * float(i) / float(4 * num);
		poly.AddVertex(Vertex(cx + r * cos(theta), cy + r * sin(theta)));
	}
	poly.AddLoopEnd(1);

	// inner loop
	for(size_t i = 0; i < 4 * num; ++i) {
		float r = 0.7 * radius;
		float theta = angle + float(2.0 * M_PI) * float(i) / float(4 * num);
		poly.AddVertex(Vertex(cx + r * cos(theta), cy + r * sin(theta)));
	}
	poly.AddLoopEnd(-1);

}

void Initialize() {

	// initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw std::runtime_error(std::string("SDL_Init error: ") + SDL_GetError());
	}

	// set opengl attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// create the main window
	g_main_window = SDL_CreateWindow("PolyMath OpenGL Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if(g_main_window == nullptr) {
		throw std::runtime_error(std::string("SDL_CreateWindow error: ") + SDL_GetError());
	}

	// create OpenGL context
	g_opengl_context = SDL_GL_CreateContext(g_main_window);
	if(g_opengl_context == nullptr) {
		throw std::runtime_error(std::string("SDL_GL_CreateContext error: ") + SDL_GetError());
	}

	// disable vsync
	SDL_GL_SetSwapInterval(0);

	// create gears
	for(size_t i = 0; i < 200; ++i) {
		AddRandomGear();
	}

}

void Cleanup() {

	// destroy OpenGL context
	if(g_opengl_context != nullptr) {
		SDL_GL_DeleteContext(g_opengl_context);
		g_opengl_context = nullptr;
	}

	// destroy the main window
	if(g_main_window != nullptr) {
		SDL_DestroyWindow(g_main_window);
		g_main_window = nullptr;
	}

	// quit SDL
	SDL_Quit();

}

void MainLoop() {

	bool wireframe = true;
	uint32_t prev_time = SDL_GetTicks();

	float fps_current;
	uint32_t fps_frames = 0;
	uint32_t fps_lasttime = prev_time;

	bool run = true;
	while(run) {

		// process events
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN: {
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE: {
							run = false;
							break;
						}
						case SDLK_f: {
							if(SDL_GetWindowFlags(g_main_window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) {
								SDL_SetWindowFullscreen(g_main_window, 0);
							} else {
								SDL_SetWindowFullscreen(g_main_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
							}
							break;
						}
						case SDLK_w: {
							wireframe = !wireframe;
							break;
						}
						case SDLK_UP: {
							AddRandomGear();
							break;
						}
						case SDLK_DOWN: {
							RemoveRandomGear();
							break;
						}
						default: break;
					}
					break;
				}
				case SDL_QUIT: {
					run = false;
					break;
				}
			}
		}

		// get window size
		int w, h;
		SDL_GL_GetDrawableSize(g_main_window, &w, &h);

		// rendering settings
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// set viewport and projection
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, w, 0, h, 1.0, -1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// draw background
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// timing
		uint32_t curr_time = SDL_GetTicks();

		// generate gears
		float ww = float(w), hh = float(h);
		float scale = sqrt(float(w) * float(h));
		float delta_time = 0.001f * float(curr_time - prev_time);
		Polygon poly;
		for(Gear &gear : g_gears) {
			gear.cx += gear.vx * delta_time;
			gear.cy += gear.vy * delta_time;
			gear.angle += gear.spin * delta_time;
			if(gear.cx < 0.0f) {
				gear.cx = -gear.cx;
				gear.vx = -gear.vx;
			}
			if(gear.cx > 1.0f) {
				gear.cx = 2.0f - gear.cx;
				gear.vx = -gear.vx;
			}
			if(gear.cy < 0.0f) {
				gear.cy = -gear.cy;
				gear.vy = -gear.vy;
			}
			if(gear.cy > 1.0f) {
				gear.cy = 2.0f - gear.cy;
				gear.vy = -gear.vy;
			}
			GenerateGearPolygon(poly, gear.cx * ww, gear.cy * hh, gear.radius * scale, gear.angle, gear.num);
		}

		// triangulate
		Polygon triangles;
		{
			PolyMath::SweepEngine<float, PolyMath::OutputPolicy_Triangles<float>, PolyMath::WindingPolicy_Positive<>, PolyMath::SweepTree_Basic2> engine(poly);
			engine.Process();
			triangles = engine.Result();
		}

		// draw triangles
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, triangles.vertices.data());
		glColor4f(1.0f, 0.5f, 0.0f, 0.4f);
		glDrawArrays(GL_TRIANGLES, 0, triangles.vertices.size());
		if(wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glColor4f(1.0f, 0.5f, 0.0f, 0.6f);
			glDrawArrays(GL_TRIANGLES, 0, triangles.vertices.size());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glDisableClientState(GL_VERTEX_ARRAY);

		// display result
		SDL_GL_SwapWindow(g_main_window);

		// timing
		prev_time = curr_time;

		// fps counter
		++fps_frames;
		if(curr_time - fps_lasttime >= 1000) {
			fps_current = float(fps_frames) / (0.001f * float(curr_time - fps_lasttime));
			std::cerr << "gears=" << g_gears.size() << ", vertices=" << poly.vertices.size() << ", fps=" << fps_current << std::endl;
			fps_frames = 0;
			fps_lasttime = curr_time;
		}

	}

}

int main(int, char**) {
	try {
		Initialize();
		MainLoop();
		Cleanup();
	} catch(...) {
		Cleanup();
		throw;
	}
	return 0;
}
