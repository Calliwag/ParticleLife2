#include "Simulation.hpp"
#include <raylib-cpp.hpp>
#include <string>

float mod(float a, float n)
{
	return fmod(fmod(a, n) + n, n);
}

void DrawLoop(Simulation* sim, map<int, Color>* typeColors, int windowX, int windowY, std::string title, int circleTexSize)
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
	SetTextureFilter(circleTex, RL_TEXTURE_FILTER_NEAREST);

	double drawRad = sim->radius / 16;

	while (!window.ShouldClose())
	{
		window.BeginDrawing();
		window.ClearBackground(BLACK);
		camera.BeginMode();
		std::for_each(std::execution::unseq, sim->particles.begin(), sim->particles.end(), [&](Particle& particle)
			{
				raylib::Vector2 pos(particle.pos.x - drawRad / 2, particle.pos.y - drawRad / 2);
				circleTex.Draw(pos, 0, drawRad / circleTexSize, (*typeColors)[particle.type]);
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

		double panSpeed = 5 / camera.zoom;
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
}

int main()
{
	SetTraceLogLevel(LOG_FATAL);

	const int numTypes = 6;

	Simulation sim;
	sim.bounds = { {0,0},{3200,3200} };
	sim.deltaTime = 0.05;
	sim.radius = 50;
	srand(time(NULL));
	int seed = rand();
	printf("Seed: %i \n", seed);
	sim.rand.seed(seed);
	sim.RandomRuleset(numTypes);
	sim.FillBounds(40000, numTypes);
	sim.friction = 5.0;
	sim.forceMult = 50;
	sim.repellMult = 1;
	sim.maxForce = sim.repellMult * sim.forceMult;
	sim.InitGrid();

	map<int, Color> typeColors = {};
	std::uniform_int_distribution<int> dist(0, 255);
	for (int i = 0; i < numTypes; i++)
	{
		Color c = BLACK;
		c.r = dist(sim.rand);
		c.g = dist(sim.rand);
		c.b = dist(sim.rand);
		double mult = 1.0 / ((c.r + c.g + c.b) / 255.0);
		c.r *= mult;
		c.g *= mult;
		c.b *= mult;
		typeColors[i] = c;
	}
	int windowX = 800;
	int windowY = 800;
	int circleSize = 4;
	std::thread drawThread([&]() {
		DrawLoop(&sim, &typeColors, windowX, windowY, "Simulation", circleSize);
		});
	int frame = 0;
	Timer timer;
	while (true)
	{
		printf("Frame: %i, FPS: %i\n", frame++, int(1 / timer()));
		sim.Step();
	}
	return 0;
}
