#include "Camera.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_projection.hpp"


void Camera::reset(float arenaWidth, float arenaHeight) {
	viewOrigin = {0.f, 0.f, arenaHeight * 0.5f};
	clippingDistance = std::max(arenaWidth, arenaHeight) * 2.f;
	zoomInv = 1.f;
}

void Camera::update(int screenWidth, int screenHeight, float arenaWidth, float arenaHeight) {
	cache.screenWidth = screenWidth;
	cache.screenHeight = screenHeight;
	cache.arenaWidth = arenaWidth;
	cache.arenaHeight = arenaHeight;

	if (arenaWidth * (float)screenHeight > (float)screenWidth * arenaHeight) { // Level is wider than screen
		halfWidth = arenaWidth * 0.5f;
		halfHeight = halfWidth * (float)screenHeight / (float)screenWidth;
	} else {
		halfHeight = arenaHeight * 0.5f;
		halfWidth = halfHeight * (float)screenWidth / (float)screenHeight;
	}
	halfWidth *= zoomInv;
	halfHeight *= zoomInv;

	projectionMatrix = glm::ortho(halfWidth, -halfWidth, -halfHeight, halfHeight, -clippingDistance, clippingDistance);
	viewMatrix = buildViewMatrix(viewRotationMatrix, viewOrigin);
}


void Camera::zoom(float amount, glm::vec2 pointerPosition) {
	float multiplier = 1.f / amount;
	zoomInv *= multiplier;
	viewOrigin = viewOrigin * multiplier + pointerToWorldPosition(pointerPosition) * (1 - multiplier);
	update(cache.screenWidth, cache.screenHeight, cache.arenaWidth, cache.arenaHeight);
}


glm::vec3 Camera::pointerToWorldPosition(glm::vec2 pointerPosition) const {
	glm::vec4 viewport = glm::vec4(0.f, 0.f, cache.screenWidth, cache.screenHeight);
	glm::vec3 screenPosition = glm::vec3(pointerPosition.x, cache.screenHeight - pointerPosition.y, 0.f);
	glm::vec3 worldPosition = glm::unProject(screenPosition, viewMatrix, projectionMatrix, viewport);
	worldPosition.x = 0.f;
	return worldPosition;
}