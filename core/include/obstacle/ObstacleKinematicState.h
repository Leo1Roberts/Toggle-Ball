#ifndef OBSTACLE_KINEMATIC_STATE_H
#define OBSTACLE_KINEMATIC_STATE_H

#include "utilities/Utilities.h"

#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"


constexpr glm::vec3 OBSTACLE_ROTATION_AXIS = {1, 0, 0};

inline glm::mat3 angleToRotation3D(float radians) {
	return glm::mat3_cast(glm::angleAxis(radians, OBSTACLE_ROTATION_AXIS));
}

class ObstacleKinematicState {
public:
	ObstacleKinematicState() = default;
	ObstacleKinematicState(glm::vec3 position, float angle, glm::vec3 velocity, float angularVelocity) :
		position(position),
		velocity(velocity),
		angularVelocity(angularVelocity) {
		setAngle(angle);
	}

	void setPosition(glm::vec3 pos) { position = pos; }
	void setAngle(float radians) {
		angle = wrapAngle(radians);
		rotation = angleToRotation3D(radians);
	}
	void setVelocity(glm::vec3 vel) { velocity = vel; }
	void setAngularVelocity(float angVel) { angularVelocity = angVel; }
	void setPhase(float radians) { phase = wrapAngle(radians); }

	[[nodiscard]] glm::vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] glm::mat3 getRotation() const { return rotation; }
	[[nodiscard]] glm::vec3 getVelocity() const { return velocity; }
	[[nodiscard]] float getAngularVelocity() const { return angularVelocity; }
	[[nodiscard]] float getPhase() const { return phase; }

private:
	glm::vec3 position{0.f};
	float angle = 0.f;
	glm::mat3 rotation = glm::mat3(1.f);
	glm::vec3 velocity{0.f};
	float angularVelocity = 0.f;
	float phase = 0.f;
};


#endif // OBSTACLE_KINEMATIC_STATE_H
