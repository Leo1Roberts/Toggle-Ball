#ifndef BALL_KINEMATIC_STATE_H
#define BALL_KINEMATIC_STATE_H

#include <glm/glm.hpp>


struct BallKinematicState {
	glm::vec3 position{0.f};
	glm::mat3 rotation{1.f};
	glm::vec3 velocity{0.f};
	glm::vec3 angularVelocity{0.f};
};

struct BallForces {
	glm::vec3 force{0.f};
	glm::vec3 torque{0.f};

	void reset() { force = torque = glm::vec3(0.f); }
};


#endif // BALL_KINEMATIC_STATE_H
