#ifndef MATRIX_UTILITIES_H
#define MATRIX_UTILITIES_H

#include <glm/glm.hpp>

glm::mat3 buildViewRotationMatrix(glm::vec3 viewDirection);

glm::mat4 buildViewMatrix(const glm::mat3& viewRotationMatrix, glm::vec3 viewPosition);

glm::mat4 buildScaledWorldMatrix(const glm::mat3& rotation, glm::vec3 position, glm::vec3 scale = glm::vec3(1.f));

#endif // MATRIX_UTILITIES_H