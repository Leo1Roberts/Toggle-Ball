#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>


struct PlaneDescriptor {
	glm::vec3 normal{0.f};
	float dotProduct{};

	PlaneDescriptor() = default;
	PlaneDescriptor(glm::vec3 normal, glm::vec3 point) : normal(normal), dotProduct(dot(point, normal)) {}
};


#endif // PLANE_H
