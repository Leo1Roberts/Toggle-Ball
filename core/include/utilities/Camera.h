#ifndef CAMERA_H
#define CAMERA_H

#include "utilities/MatrixUtilities.h"

#include <glm/glm.hpp>


class Camera {
public:
	void reset(float arenaWidth, float arenaHeight);
	void update(float screenWidth, float screenHeight, float arenaWidth, float arenaHeight);

	void startPan(glm::vec2 pointerPosition);
	void updatePan(glm::vec2 pointerPosition);
	void zoom(float amount, glm::vec2 pointerPosition);

	[[nodiscard]] glm::vec2 screenToPlanarPosition(glm::vec2 screenPosition) const;
	[[nodiscard]] glm::vec2 planarToScreenPosition(glm::vec2 planarPosition) const;
	static glm::vec2 planarToScreenDirection(glm::vec2 planarDirection); // Magnitude is not considered

	[[nodiscard]] float getHalfHeight() const { return halfHeight; }
	[[nodiscard]] const glm::mat4& getViewMatrix() const { return viewMatrix; }
	[[nodiscard]] const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

	[[nodiscard]] const glm::mat3& getViewRotationMatrix() const { return viewRotationMatrix; }
	[[nodiscard]] const glm::mat3& getWorldToViewRotationMatrix() const { return worldToViewRotationMatrix; }

private:
	glm::vec3 viewOrigin{};
	glm::vec2 panPointerPosition{};
	float zoomInv = 1.f;

	float clippingDistance{};
	float halfWidth{}, halfHeight{};

	glm::mat4 viewMatrix{}, projectionMatrix{};

	struct {
		float
		screenWidth{}, screenHeight{},
		arenaWidth{}, arenaHeight{};
	} cache;

	static constexpr glm::vec3 viewDirection = {-1, 0, 0};
	const glm::mat3 viewRotationMatrix = buildViewRotationMatrix(viewDirection);
	const glm::mat3 worldToViewRotationMatrix = glm::transpose(viewRotationMatrix);
};


#endif // CAMERA_H
