#include "main.h"
#include "Smoother.h"
#include <algorithm>

void Smoother::Reset() {
	memset(this, 0, sizeof(Smoother));
}

void Smoother::SetPosition(float x, float v) {
	memset(this, 0, sizeof(Smoother));

	S = X = D = x;
	V = DestV = v;
}

void Smoother::SetDestination(float dest, float dest_v, float time) {
	S = X;
	U = V;
	D = dest;
	DestV = dest_v;
	Arrive = time;
	Elapsed = 0.0f;

	mat3 M, inverse;

	float
			t_squared_over_2 = Arrive * Arrive / 2.0f,
			t_cubed_over_6 = (Arrive * t_squared_over_2) / 3.0f,
			t_to_4_over_24 = t_squared_over_2 * t_squared_over_2 / 6.0f;

	// use matrix to solve simultaneous equations with 3 unknowns

	M.a = t_to_4_over_24;
	M.b = t_cubed_over_6;
	M.c = t_squared_over_2;
	M.d = t_cubed_over_6;
	M.e = t_squared_over_2;
	M.f = Arrive;
	M.g = t_squared_over_2;
	M.h = Arrive;
	M.i = 1.0f;

	inverse.SetInverseOf(&M);

	vec3 vector, res;

	vector.x = D - S - U * Arrive;
	vector.y = dest_v - U;
	vector.z = 0.0f;

	res = inverse * vector;

	K = res.x;
	R = res.y;
	I = res.z;
}

void Smoother::Update(float f_t_inc) {
	Elapsed += f_t_inc;

	if (Elapsed >= Arrive) // terminating condition
	{
		X = D;
		V = DestV;
		A = 0.0f;
		Elapsed = Arrive;
		return;
	}

	float
			t_squared_over_2 = Elapsed * Elapsed / 2.0f,
			t_cubed_over_6 = (Elapsed * t_squared_over_2) / 3.0f,
			t_to_4_over_24 = t_squared_over_2 * t_squared_over_2 / 6.0f;

	V = K * t_cubed_over_6 + R * t_squared_over_2 + I * Elapsed + U;
	X = std::clamp(K * t_to_4_over_24 + R * t_cubed_over_6 + I * t_squared_over_2 + U * Elapsed + S, 0.0f, 1.0f);
}