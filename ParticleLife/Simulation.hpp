#pragma once

#include "Particle.hpp"
#include <map>
#include <vector>
#include <random>
#include <mutex>

using std::map;
using std::vector;
using std::mutex;

class Cell
{
public:
	vector<Particle*> particles = {};
	//SpinLock lock;

	Cell()
	{
		particles.reserve(25);
	}
	Cell* operator=(const Cell&)
	{
		particles = {};
		return this;
	}
	void Clear()
	{
		particles.reserve(100);
		particles.clear();
	}
};

class Simulation
{
public:
	map<int,TypeInfo> types;
	vector<Particle> particles = {};
	RectD bounds;
	Grid<Cell> cells;
	double radius;
	double closeRadius;
	std::mt19937* rand;

	void Step();
	void UpdateGrid();
	Vec2i GridPos(Vec2d pos);
	void CalculateInteractions();
	void UpdateParticles();
	void FillBounds(int count, int typeCount);
	void InitGrid();
};