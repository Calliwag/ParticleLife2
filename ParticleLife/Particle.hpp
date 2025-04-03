#pragma once

#include "Types.hpp"
#include <raylib-cpp.hpp>

struct ColorPalette
{
	raylib::Color base = GREEN;
	raylib::Color highCount = YELLOW;
	raylib::Color midCount = BLUE;
	raylib::Color lowCount = BROWN;
	raylib::Color closeCount = MAGENTA;

	ColorPalette() {};
	ColorPalette(raylib::Color Base, raylib::Color High, raylib::Color Mid, raylib::Color Low, raylib::Color Close) : base(Base), highCount(High), midCount(Mid), lowCount(Low), closeCount(Close) {};
};

class TypeInfo
{
public:
	double fixedRotation;
	double forceRotation;
	double moveDist;
	double mass;
	double ratio;
	ColorPalette palette;

	TypeInfo(double FixedRotation, double ForceRotation, double MoveDist, double Mass, double Ratio) : fixedRotation(FixedRotation), forceRotation(ForceRotation), moveDist(MoveDist), mass(Mass), ratio(Ratio) {};
	TypeInfo() {};
};

class Particle
{
public:
	Vec2d pos = { 0,0 };
	float angle = 0;
	float dAngle = 0;
	float count = 0;
	float closeCount = 0;
	int type = -1;

	Particle() {};
	Particle(const Particle& other)
	{
		pos = other.pos;
		angle = other.angle;
		type = other.type;
	}
	void Update(TypeInfo& typeInfo);
};