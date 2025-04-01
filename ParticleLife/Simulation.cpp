#include "Simulation.hpp"
#include <execution>

void Simulation::Step()
{
	UpdateGrid();
	CalculateInteractions();
	UpdateParticles();
}

void Simulation::UpdateGrid()
{
	for (int i = 0; i < cells.X * cells.Y; i++)
	{
		cells.arr[i].Clear();
	}
	for (int i = 0; i < particles.size(); i++)
	{
		Vec2d pos = particles[i].pos + particles[i].vel;
		Vec2i gridPos = GridPos(pos);
		cells[gridPos].particles.push_back(&particles[i]);
	}
}

Vec2i Simulation::GridPos(Vec2d pos)
{
	double fracX = fmin(0.999999, (pos.x - bounds.c1.x) / (bounds.c2.x - bounds.c1.x));
	double fracY = fmin(0.999999, (pos.y - bounds.c1.y) / (bounds.c2.y - bounds.c1.y));
	Vec2i gridPos = { fracX * cells.X,fracY * cells.Y};
	gridPos.x = (gridPos.x + cells.X) % cells.X;
	gridPos.y = (gridPos.y + cells.Y) % cells.Y;
	return gridPos;
}

void Simulation::CalculateInteractions()
{
	double spanX = bounds.c2.x - bounds.c1.x;
	double spanY = bounds.c2.y - bounds.c1.y;
	std::for_each(std::execution::par_unseq, particles.begin(), particles.end(), [&](Particle& particle)
		{
			Vec2d pos = particle.pos + particle.vel;
			Vec2i gridPos = GridPos(pos);
			for(int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					Vec2i _gridPos = (gridPos + Vec2i{ x,y });
					_gridPos.x = (_gridPos.x + cells.X) % cells.X;
					_gridPos.y = (_gridPos.y + cells.Y) % cells.Y;
					vector<Particle*>& particles = cells[_gridPos].particles;
					for (int i = 0; i < particles.size(); i++)
					{
						Particle& other = *particles[i];
						if (&particle == &other)
							continue;
						Vec2d pos1 = pos;
						Vec2d fixedPos1 = { 0,0 };
						Vec2d pos2 = other.pos + other.vel;
						double smallestDist = std::numeric_limits<double>::max();
						for (int x = -1; x <= 1; x++)
							for (int y = -1; y <= 1; y++)
							{
								Vec2d _pos1 = pos1;
								_pos1.x += x * spanX;
								_pos1.y += y * spanY;
								Vec2d _axis = pos2 - _pos1;
								double _dist = _axis.x * _axis.x + _axis.y * _axis.y;
								if (_dist < smallestDist)
								{
									smallestDist = _dist;
									fixedPos1 = _pos1;
								}
							}
						pos1 = fixedPos1;
						Vec2d axis = pos2 - pos1;
						double dist = Mag(axis);
						if (dist > radius)
							continue;
						double mult = ruleset[{particle.type, other.type}];
						axis *= (1 / dist);
						double frac = dist / radius;
						double force = 0;
						if (frac < 1 / 3.0)
						{
							frac = 3 * frac;
							force = repellMult * (frac - 1);
						}
						else if (frac < 2 / 3.0)
						{
							frac = 3 * (frac - 1 / 3.0);
							force = mult * frac;
						}
						else
						{
							frac = 3 * (frac - 2 / 3.0);
							force = mult * (1 - frac);
						}
						force *= forceMult;
						if (force > maxForce) force = maxForce;
						if (force < -maxForce) force = -maxForce;
						particle.acc += force * axis;
					}
				}
			}
		});
}

void Simulation::UpdateParticles()
{
	for (int i = 0; i < particles.size(); i++)
	{
		Particle& particle = particles[i];
		particle.Update(deltaTime);
		particle.vel -= friction * particle.vel * deltaTime;
		if (particle.pos.x < bounds.c1.x)
		{
			particle.pos.x += bounds.c2.x - bounds.c1.x;
		}
		else if (particle.pos.x >= bounds.c2.x)
		{
			particle.pos.x += bounds.c1.x - bounds.c2.x;
		}
		if (particle.pos.y < bounds.c1.y)
		{
			particle.pos.y += bounds.c2.y - bounds.c1.y;
		}
		else if (particle.pos.y >= bounds.c2.y)
		{
			particle.pos.y += bounds.c1.y - bounds.c2.y;
		}
	}
}

void Simulation::FillBounds(int count, int types)
{
	std::uniform_real_distribution<double> distX(bounds.c1.x, bounds.c2.x);
	std::uniform_real_distribution<double> distY(bounds.c1.y, bounds.c2.y);
	std::uniform_real_distribution<double> distV(-1, 1);
	std::uniform_int_distribution<int> distT(0, types - 1);
	for (int i = 0; i < count; i++)
	{
		Vec2d pos(distX(rand), distY(rand));
		Particle particle;
		particle.pos = pos;
		particle.vel = { distV(rand) * deltaTime,distV(rand) * deltaTime};
		particle.acc = { 0,0 };
		particle.type = distT(rand);
		particles.push_back(particle);
	}
}

void Simulation::RandomRuleset(int types)
{
	ruleset = Grid<double>(types, types);
	std::uniform_real_distribution<double> distF(-1, 1);
	for (int i = 0; i < types; i++)
	{
		for (int j = 0; j < types; j++)
		{
			ruleset[{i, j}] = distF(rand);
		}
	}
}

void Simulation::InitGrid()
{
	int sizeX = (bounds.c2.x - bounds.c1.x) / radius;
	int sizeY = (bounds.c2.y - bounds.c1.y) / radius;

	cells = Grid<Cell>(sizeX, sizeY);
}