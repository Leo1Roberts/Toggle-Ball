#ifndef PLANE_H
#define PLANE_H
#include "VectorMatrix.h"


struct PlaneDescriptor {
	vec3 normal;
	float dotProduct{};

	PlaneDescriptor() = default;
	PlaneDescriptor(vec3 normal, vec3 point) : normal(normal), dotProduct(dot(point, normal)) {}
};


#endif // PLANE_H
