#include "Simulation.hpp"
#include <raylib-cpp.hpp>
#include <string>
#include <execution>
#include <atomic>

using std::atomic_bool;

Color operator*(const Color& color, double scale)
{
	if (scale > 1) scale = 1;
	return { (unsigned char)(color.r * scale),(unsigned char)(color.g * scale),(unsigned char)(color.b * scale),color.a };
}

void DrawLoop(atomic_bool* running, Simulation* sim, int windowX, int windowY, std::string title, int circleTexSize)
{
	raylib::Window window(windowX, windowY, title);
	float spanX = sim->bounds.c2.x - sim->bounds.c1.x;
	float spanY = sim->bounds.c2.y - sim->bounds.c1.y;
	raylib::Camera2D camera(Vector2{ windowX / 2.f, windowY / 2.f }, Vector2{ spanX / 2.f, spanY / 2.f }, 0, windowX / spanX);
	window.SetTargetFPS(60);

	raylib::RenderTexture circleRender(circleTexSize, circleTexSize);
	circleRender.BeginMode();
	raylib::Vector2 pos = { (float)circleTexSize / 2,(float)circleTexSize / 2};
	pos.DrawCircle(circleTexSize / 2, WHITE);
	circleRender.EndMode();
	raylib::Texture circleTex = circleRender.GetTexture();
	//SetTextureFilter(circleTex, RL_TEXTURE_FILTER_TRILINEAR);

	double drawRad = sim->radius / 8;
	raylib::Rectangle sourceRect(0, 0, circleTexSize, circleTexSize);
	raylib::Rectangle destRect(0, 0, drawRad, drawRad);
	raylib::Vector2 origin(0, 0);
	double panSpeed = 5 / camera.zoom;

	running->store(true);
	running->notify_one();
	while (!window.ShouldClose())
	{
		//SetTraceLogLevel(LOG_FATAL);
		window.BeginDrawing();
		window.ClearBackground(BLACK);
		camera.BeginMode();
		std::for_each(std::execution::seq, sim->particles.begin(), sim->particles.end(), [&](Particle& particle)
			{
				destRect.x = particle.pos.x - drawRad / 2;
				destRect.y = particle.pos.y - drawRad / 2;
				Color color = WHITE;
				double n = particle.count;
				TypeInfo& info = sim->types[particle.type];
				ColorPalette& palette = info.palette;
				if (n > 15 && n <= 35)
					color = palette.midCount;
				else if (n > 35)
					color = palette.highCount;
				else if (n >= 13 && n <= 15)
					color = palette.lowCount;
				else if (particle.closeCount > 15)
					color = palette.closeCount;
				else
					color = palette.base;
					
				circleTex.Draw(sourceRect, destRect, origin, 0, color/* * (particle.count / 5.0)*/);
			});
		raylib::Rectangle rlBounds;
		rlBounds.x = sim->bounds.c1.x;
		rlBounds.y = sim->bounds.c1.y;
		rlBounds.width = sim->bounds.c2.x - sim->bounds.c1.x;
		rlBounds.height = sim->bounds.c2.y - sim->bounds.c1.y;
		rlBounds.DrawLines(WHITE);
		camera.EndMode();
		window.DrawFPS();
		window.EndDrawing();

		if (raylib::Keyboard::IsKeyDown(KEY_UP))
		{
			camera.target.y -= panSpeed;
		}
		if (raylib::Keyboard::IsKeyDown(KEY_DOWN))
		{
			camera.target.y += panSpeed;
		}
		if (raylib::Keyboard::IsKeyDown(KEY_LEFT))
		{
			camera.target.x -= panSpeed;
		}
		if (raylib::Keyboard::IsKeyDown(KEY_RIGHT))
		{
			camera.target.x += panSpeed;
		}
		if (raylib::Keyboard::IsKeyDown(KEY_LEFT_BRACKET))
		{
			camera.zoom *= 0.95;
		}
		if (raylib::Keyboard::IsKeyDown(KEY_RIGHT_BRACKET))
		{
			camera.zoom /= 0.95;
		}
		camera.offset = { window.GetWidth() / 2.f, window.GetHeight() / 2.f };
	}
	running->store(false);
	running->notify_one();
}

int main()
{
	Simulation sim;
	sim.bounds = { {0,0},{400,400} };
	sim.radius = 5;
	sim.closeRadius = 1.3;
	srand(time(NULL));
	int seed = rand();
	printf("Seed: %i \n", seed);
	sim.rand = new std::mt19937(seed);

	const int numTypes = 2;
	sim.types[0] = TypeInfo(180 * (T_Pi / 180), 17 * (T_Pi / 180), 0.134 * sim.radius, 1, 0.9);
	sim.types[0].palette = ColorPalette({ 0,128,0 }, { 0,128,255 }, { 0,128,128 }, { 0,128,64 }, { 0,255,0 });
	sim.types[1] = TypeInfo(180 * (T_Pi / 180), 3 * (T_Pi / 180), 0.134 * sim.radius, -1, 0.1);
	sim.types[1].palette = ColorPalette({ 128,0,0 }, { 222,64,0 }, { 192,32,128 }, { 156,16,64 }, { 255,64,0 });

	sim.FillBounds(0.12 * sim.bounds.Area(), numTypes);
	sim.InitGrid();

	int windowX = 800;
	int windowY = 800;
	int circleSize = 16;
	atomic_bool running{ false };
	std::thread drawThread([&]() {
		DrawLoop(&running, &sim, windowX, windowY, "Simulation", circleSize);
		});
	running.wait(false);
	int frame = 0;
	Timer timer;
	double targetFPS = 30;
	bool capFPS = false;
	while (running.load())
	{
		sim.Step();
		double time = timer();
		if(capFPS)
		{
			WaitTime(std::max(0.0, 1.0 / targetFPS - time));
			time += timer();
		}
		printf("Frame: %i, FPS: %i\n", frame++, int(1.0 / time));
	}
	return 0;
}
