#include "Simulation.hpp"
#include <raylib-cpp.hpp>

int main()
{
	SetTraceLogLevel(LOG_FATAL);

	const int numTypes = 5;

	Simulation sim;
	sim.bounds = { {0,0},{800,800} };
	sim.deltaTime = 0.1;
	sim.radius = 25;
	srand(time(NULL));
	int seed = rand();
	seed =
		//6651;
		18900;
		//seed;
	printf("Seed: %i \n", seed);
	sim.rand.seed(seed);
	sim.RandomRuleset(numTypes);
	sim.FillBounds(2000, numTypes);
	sim.friction = .05;
	sim.forceMult = 10;
	sim.repellMult = 1;

	map<int, Color> typeColors = {};
	std::uniform_int_distribution<int> dist(0, 255);
	for (int i = 0; i < numTypes; i++)
	{
		Color c = BLACK;
		c.r = dist(sim.rand);
		c.g = dist(sim.rand);
		c.b = dist(sim.rand);
		double mult = 1.0 / (sqrt(c.r * c.r + c.g * c.g + c.b * c.b) / 255.0);
		c.r *= mult;
		c.g *= mult;
		c.b *= mult;
		typeColors[i] = c;
	}

	raylib::Window window(800, 800, "Particle Life");

	window.SetTargetFPS(60);

	sim.InitGrid();
	while (!window.ShouldClose())
	{
		sim.Step();

		window.BeginDrawing();
		window.ClearBackground(BLACK);

		for (int i = 0; i < sim.particles.size(); i++)
		{
			Particle& particle = sim.particles[i];
			raylib::Vector2 pos(particle.pos.x, particle.pos.y);
			pos.DrawCircle(sim.radius / 6, typeColors[particle.type]);
		}

		window.EndDrawing();
	}
	return 0;
}
