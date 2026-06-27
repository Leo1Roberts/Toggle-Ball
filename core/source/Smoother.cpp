#include "main.h"
#include "Smoother.h"
#include <algorithm>

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
	elapsed = 0.0f;

	mat3 M, inverse;

	float
		t_squared_over_2 = arrive * arrive / 2.0f,
		t_cubed_over_6 = (arrive * t_squared_over_2) / 3.0f,
		t_to_4_over_24 = t_squared_over_2 * t_squared_over_2 / 6.0f;

	// use matrix to solve simultaneous equations with 3 unknowns

	M.a = t_to_4_over_24;
	M.b = t_cubed_over_6;
	M.c = t_squared_over_2;
	M.d = t_cubed_over_6;
	M.e = t_squared_over_2;
	M.f = arrive;
	M.g = t_squared_over_2;
	M.h = arrive;
	M.i = 1.0f;

	inverse.SetInverseOf(&M);

	vec3 vector, res;

	vector.x = d - s - u * arrive;
	vector.y = destVel - u;
	vector.z = 0.0f;

	res = inverse * vector;

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
			t_squared_over_2 = elapsed * elapsed / 2.0f,
			t_cubed_over_6 = (elapsed * t_squared_over_2) / 3.0f,
			t_to_4_over_24 = t_squared_over_2 * t_squared_over_2 / 6.0f;

	v = k * t_cubed_over_6 + r * t_squared_over_2 + i * elapsed + u;
	x = std::clamp(k * t_to_4_over_24 + r * t_cubed_over_6 + i * t_squared_over_2 + u * elapsed + s, 0.0f, 1.0f);
}