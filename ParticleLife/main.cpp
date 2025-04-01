#include "Simulation.hpp"
#include <raylib-cpp.hpp>

float mod(float a, float n)
{
	return fmod(fmod(a, n) + n, n);
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
	raylib::Window window(800, 800, "Particle Life");
	float spanX = sim.bounds.c2.x - sim.bounds.c1.x;
	float spanY = sim.bounds.c2.y - sim.bounds.c1.y;
	raylib::Camera2D camera(Vector2{ window.GetWidth() / 2.f, window.GetHeight() / 2.f }, Vector2{ spanX / 2.f, spanY / 2.f }, 0, window.GetWidth() / (sim.bounds.c2.x - sim.bounds.c1.x));
	//window.SetTargetFPS(60);

	const int circleTexSize = 4;
	raylib::RenderTexture circleRender(circleTexSize, circleTexSize);
	circleRender.BeginMode();
	raylib::Vector2 pos = { circleTexSize / 2,circleTexSize / 2 };
	pos.DrawCircle(circleTexSize / 2, WHITE);
	circleRender.EndMode();
	raylib::Texture circleTex = circleRender.GetTexture();
	SetTextureFilter(circleTex, RL_TEXTURE_FILTER_NEAREST);

	int frame = 0;
	while (!window.ShouldClose())
	{
		printf("Frame: %i, FPS: %i\n", frame++, window.GetFPS());
		sim.Step();

		window.BeginDrawing();
		window.ClearBackground(BLACK);
		camera.BeginMode();
		for (int i = 0; i < sim.particles.size(); i++)
		{
			Particle& particle = sim.particles[i];
			raylib::Vector2 pos(particle.pos.x - sim.radius / 32, particle.pos.y - sim.radius / 32);
			circleTex.Draw(pos,0,sim.radius / (16 * circleTexSize), typeColors[particle.type]);
		}
		raylib::Rectangle rlBounds;
		rlBounds.x = sim.bounds.c1.x;
		rlBounds.y = sim.bounds.c1.y;
		rlBounds.width = sim.bounds.c2.x - sim.bounds.c1.x;
		rlBounds.height = sim.bounds.c2.y - sim.bounds.c1.y;
		rlBounds.DrawLines(WHITE);
		camera.EndMode();
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
	return 0;
}
