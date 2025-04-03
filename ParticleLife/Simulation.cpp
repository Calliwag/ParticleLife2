#include "Simulation.hpp"
#include <execution>

inline float mmod(float a, float n)
{
	return fmod(fmod(a, n) + n, n);
}

inline int dsign(double x) {
	return (x < 0) ? -1 : (x > 0) ? 1 : 0;
}

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
		Particle& particle = particles[i];
		Vec2i gridPos = GridPos(particle.pos);
		//cells[gridPos].lock.Lock();
		cells[gridPos].particles.push_back(&particle);
		//cells[gridPos].lock.Unlock();
	}
	//for (int i = 0; i < cells.X * cells.Y; i++)
	//{
	//	cells.arr[i].Clear();
	//	//cells.arr[i].particles.reserve(100);
	//}
	//std::for_each(std::execution::unseq, particles.begin(), particles.end(), [&](Particle& particle)
	//	{
	//		Vec2d pos = particle.pos;
	//		Vec2i gridPos = GridPos(pos);
	//		Cell& cell = cells[gridPos];
	//		//cell.lock.Lock();
	//		cell.particles.push_back(&particle);
	//		//cell.lock.Unlock();
	//	});
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
	double radiusSqr = radius * radius;
	double closeRadiusSqr = closeRadius * closeRadius;
	std::for_each(std::execution::par, particles.begin(), particles.end(), [&](Particle& particle)
		{
			particle.count = 0;
			particle.closeCount = 0;
			Vec2i gridPos = GridPos(particle.pos);
			float left = 0;
			float right = 0;
			auto& cellParticles = cells[gridPos].particles;
			Vec2d thisPos = particle.pos;
			Vec2d rightNorm = { sin(particle.angle),-cos(particle.angle)};
			bool fixPos = (gridPos.x == 0)
				|| (gridPos.y == 0)
				|| (gridPos.x == cells.X - 1)
				|| (gridPos.y == cells.Y - 1);
			for(int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					Vec2i _gridPos = {
						(x + gridPos.x + cells.X) % cells.X,
						(y + gridPos.y + cells.Y) % cells.Y
					};
					Vec2d offset = { 0,0 };
					if(fixPos)
					{
						if (_gridPos.x - gridPos.x > 1)
						{
							offset.x = -spanX;
						}
						else if (_gridPos.x - gridPos.x < -1)
						{
							offset.x = spanX;
						}
						if (_gridPos.y - gridPos.y > 1)
						{
							offset.y = -spanY;
						}
						else if (_gridPos.y - gridPos.y < -1)
						{
							offset.y = spanY;
						}
					}
					vector<Particle*>& cellParticles = cells[_gridPos].particles;
					for (int i = 0; i < cellParticles.size(); i++)
					{
						Particle& other = *cellParticles[i];
						if (&particle == &other)
							continue;
						Vec2d axis = other.pos - thisPos;
						if(fixPos)
							axis += offset;
						if (axis.x > radius || axis.y > radius)
							continue;
						double distSqr = axis.x * axis.x + axis.y * axis.y;
						if (distSqr <= radiusSqr)
						{
							particle.count++;
							if (distSqr < closeRadiusSqr)
								particle.closeCount++;
							Dot(axis, rightNorm) >= 0 ? right += types[other.type].mass : left += types[other.type].mass;
						}
					}
				}
			}
			float total = left + right;
			float difference = left - right;
			TypeInfo type = types[particle.type];
			particle.dAngle += type.fixedRotation + type.forceRotation * total * dsign(difference);
		});
}

void Simulation::UpdateParticles()
{
	std::for_each(std::execution::par_unseq, particles.begin(), particles.end(), [&](Particle& particle)
		{
			particle.Update(types[particle.type]);
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
		});
}

void Simulation::FillBounds(int count, int typeCount)
{
	std::uniform_real_distribution<double> distX(bounds.c1.x, bounds.c2.x);
	std::uniform_real_distribution<double> distY(bounds.c1.y, bounds.c2.y);
	std::uniform_real_distribution<double> distA(0, 2 * T_Pi);
	double sumRatio = 0;
	for (int i = 0; i < typeCount; i++)
	{
		sumRatio += types[i].ratio;
	}
	std::uniform_real_distribution<double> distT(0, sumRatio);
	for (int i = 0; i < count; i++)
	{
		Vec2d pos(distX(*rand), distY(*rand));
		Particle particle;
		particle.pos = pos;
		particle.angle = distA(*rand);
		double t = distT(*rand);
		for (int j = 0; j < typeCount; j++)
		{
			if (t <= types[j].ratio)
			{
				particle.type = j;
				break;
			}
			else
			{
				t -= types[j].ratio;
			}
		}
		particles.push_back(particle);
	}
}

void Simulation::InitGrid()
{
	int sizeX = (bounds.c2.x - bounds.c1.x) / radius;
	int sizeY = (bounds.c2.y - bounds.c1.y) / radius;

	cells = Grid<Cell>(sizeX, sizeY);
}