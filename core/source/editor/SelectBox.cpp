#include "editor/SelectBox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

bool SelectBox::touchesCircle(glm::vec2 centre, float radius) const {
	glm::vec2 closestPoint = glm::clamp(centre, {left, bottom}, {right, top});
	glm::vec2 delta = centre - closestPoint;
	return length2(delta) <= (radius * radius);
}