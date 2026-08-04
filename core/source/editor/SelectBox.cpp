#include "editor/SelectBox.h"

#include "glm/gtx/norm.inl"

bool SelectBox::touchesCircle(glm::vec2 centre, float radius) const {
	glm::vec2 closestPoint = glm::clamp(centre, {left, bottom}, {right, top});
	glm::vec2 delta = centre - closestPoint;
	return length2(delta) <= (radius * radius);
}