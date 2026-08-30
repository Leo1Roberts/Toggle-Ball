#include "utilities/Camera.h"

#include "utilities/Utilities.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_projection.hpp"


void Camera::reset(float arenaWidth, float arenaHeight) {
	viewOrigin = {0.f, 0.f, arenaHeight * 0.5f};
	clippingDistance = std::max(arenaWidth, arenaHeight) * 2.f;
	zoomInv = 1.f;
}

void Camera::update(float screenWidth, float screenHeight, float arenaWidth, float arenaHeight, Rectangle viewport) {
	cache.screenWidth = screenWidth;
	cache.screenHeight = screenHeight;
	cache.arenaWidth = arenaWidth;
	cache.arenaHeight = arenaHeight;
	cache.viewport = viewport;

	if (viewport.width() == 0.f || viewport.height() == 0.f) return;

	if (arenaWidth * viewport.height() > viewport.width() * arenaHeight) { // Level is wider than screen
		halfWidth = arenaWidth * 0.5f;
		halfHeight = halfWidth * viewport.height() / viewport.width();
	} else {
		halfHeight = arenaHeight * 0.5f;
		halfWidth = halfHeight * viewport.width() / viewport.height();
	}
	halfWidth *= zoomInv;
	halfHeight *= zoomInv;

	glm::vec2 centre = {
		viewport.x() + (viewport.width() * 0.5f),
		viewport.y() + (viewport.height() * 0.5f)};

	float unitsPerPixel = (halfWidth * 2.f) / viewport.width();

	float
	left   = centre.x * unitsPerPixel,
	right  = -(screenWidth - centre.x) * unitsPerPixel,
	bottom = -centre.y * unitsPerPixel,
	top    = (screenHeight - centre.y) * unitsPerPixel;

	projectionMatrix = glm::ortho(left, right, bottom, top, -clippingDistance, clippingDistance);
	viewMatrix = buildViewMatrix(viewRotationMatrix, viewOrigin);
}


void Camera::startPan(glm::vec2 pointerPosition) {
	panPointerPosition = pointerPosition;
}
void Camera::updatePan(glm::vec2 pointerPosition) {
	glm::vec2 panPointerPlanarPosition = screenToPlanarPosition(panPointerPosition);
	glm::vec2 pointerPlanarPosition = screenToPlanarPosition(pointerPosition);
	viewOrigin -= planarToWorld(pointerPlanarPosition - panPointerPlanarPosition);
	panPointerPosition = pointerPosition;
	update(cache.screenWidth, cache.screenHeight, cache.arenaWidth, cache.arenaHeight, cache.viewport);
}

void Camera::zoom(float amount, glm::vec2 pointerPosition) {
	float multiplier = 1.f / amount;
	zoomInv *= multiplier;
	viewOrigin = viewOrigin * multiplier + planarToWorld(screenToPlanarPosition(pointerPosition) * (1 - multiplier));
	update(cache.screenWidth, cache.screenHeight, cache.arenaWidth, cache.arenaHeight, cache.viewport);
}


glm::vec2 Camera::screenToPlanarPosition(glm::vec2 screenPosition) const {
	glm::vec4 viewport = glm::vec4(0.f, 0.f, cache.screenWidth, cache.screenHeight);
	glm::vec3 flippedScreenPosition = glm::vec3(screenPosition.x, cache.screenHeight - screenPosition.y, 0.f);
	return worldToPlanar(glm::unProject(flippedScreenPosition, viewMatrix, projectionMatrix, viewport));;
}
glm::vec2 Camera::planarToScreenPosition(glm::vec2 planarPosition) const {
	glm::vec4 viewport = glm::vec4(0.f, 0.f, cache.screenWidth, cache.screenHeight);
	glm::vec3 projectedPosition = glm::project(planarToWorld(planarPosition), viewMatrix, projectionMatrix, viewport);
	return { projectedPosition.x, cache.screenHeight - projectedPosition.y };
}
float Camera::planarToScreenDistance(float planarDistance) const {
	float orthoWorldWidth = 2.f / projectionMatrix[0][0];
	float pixelsPerUnit = cache.screenWidth / orthoWorldWidth;
	return std::abs(planarDistance * pixelsPerUnit);
}
glm::vec2 Camera::planarToScreenDirection(glm::vec2 planarDirection) {
	return {planarDirection.x, -planarDirection.y};
}