#include "utilities/Smoother.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <cstring>

void Smoother::reset() {
	memset(this, 0, sizeof(Smoother));
}

void Smoother::setPosition(float pos, float vel) {
	memset(this, 0, sizeof(Smoother));

	s = x = d = pos;
	v = destV = vel;
}

void Smoother::setDestination(float destPos, float destVel, float time) {
	s = x;
	u = v;
	d = destPos;
	destV = destVel;
	arrive = time;
	elapsed = 0.f;

	float
		t_squared_over_2 = arrive * arrive / 2.f,
		t_cubed_over_6 = (arrive * t_squared_over_2) / 3.f,
		t_to_4_over_24 = t_squared_over_2 * t_squared_over_2 / 6.f;

	// use matrix to solve simultaneous equations with 3 unknowns
	auto M = glm::mat3(
		t_to_4_over_24,   t_cubed_over_6,   t_squared_over_2,
		t_cubed_over_6,   t_squared_over_2, arrive,
		t_squared_over_2, arrive,           1.f
	);

	glm::mat3 inverse = glm::inverse(M);

	glm::vec3 vector(
		d - s - u * arrive,
		destVel - u,
		vector.z = 0.f
	);

	glm::vec3 res = inverse * vector;

	k = res.x;
	r = res.y;
	i = res.z;
}

void Smoother::update(float dt) {
	elapsed += dt;

	if (elapsed >= arrive) // terminating condition
	{
		x = d;
		v = destV;
		a = 0.0f;
		elapsed = arrive;
		return;
	}

	float
		t_squared_over_2 = elapsed * elapsed / 2.f,
		t_cubed_over_6 = (elapsed * t_squared_over_2) / 3.f,
		t_to_4_over_24 = t_squared_over_2 * t_squared_over_2 / 6.f;

	v = k * t_cubed_over_6 + r * t_squared_over_2 + i * elapsed + u;
	x = std::clamp(k * t_to_4_over_24 + r * t_cubed_over_6 + i * t_squared_over_2 + u * elapsed + s, 0.f, 1.f);
}