#pragma once

#include "Particle.hpp"
#include <map>
#include <vector>
#include <random>

using std::map;
using std::vector;

class Ruleset
{
	map<int, map<int, double>> rules = {};

public:
	double operator ()(int p1, int p2);
	void Set(int p, map<int, double> ruleset);
};

class Cell
{
public:
	vector<Particle*> particles = {};
	void Clear()
	{
		particles.clear();
	}
};

class Simulation
{
public:
	Ruleset ruleset;
	vector<Particle> particles = {};
	RectD bounds;
	Grid<Cell> cells;
	double deltaTime = 1;
	double radius;
	double friction;
	double forceMult;
	double repellMult;
	std::mt19937 rand;

	void Step();
	void UpdateGrid();
	Vec2i GridPos(Vec2d pos);
	void CalculateInteractions();
	void UpdateParticles();
	void FillBounds(int count, int types);
	void RandomRuleset(int types);
	void InitGrid();
};