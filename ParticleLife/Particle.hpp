#pragma once

#include "Types.hpp"
#include <mutex>

class Particle
{
public:
	Vec2d pos = { 0,0 };
	Vec2d vel = { 0,0 };
	Vec2d acc = { 0,0 };
	double mass = 1;
	bool active = true;
	int type = -1;
	std::mutex accMutex = {};
	std::mutex pvMutex = {};

	Particle() {};
	Particle(const Particle& other)
	{
		pos = other.pos;
		vel = other.vel;
		acc = other.acc;
		mass = other.mass;
		active = other.active;
		type = other.type;
	}
	void Update(double timestep);
};