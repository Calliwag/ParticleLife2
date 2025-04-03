#include "Particle.hpp"

void Particle::Update(TypeInfo& typeInfo)
{
	Vec2d dir = { cos(angle),sin(angle) };
	pos += dir * typeInfo.moveDist;
	angle += dAngle;
	angle = fmod(fmod(angle, 2 * T_Pi) + 2 * T_Pi, 2 * T_Pi);
	dAngle = 0;
}
