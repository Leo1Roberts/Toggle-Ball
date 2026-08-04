#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

enum {
	MAT_BASKETBALL,
	MAT_CONCRETE,
	MAT_NUM
};

constexpr float FRICTION_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
	0.5f,	0.58f,
	0.58f,	0.4f
};

constexpr float ROLLING_RESISTANCE_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
	0.02f,	0.015f,
	0.015f,	0.01f
};

constexpr float GRAVITY = -9.81f;
constexpr float AIR_DENSITY = 1.225f;
//const float DYNAMIC_VISCOSITY = 0.000018f;

constexpr float PHYSICS_TIMESTEP = 0.001f;

#endif // PHYSICS_CONSTANTS_H
