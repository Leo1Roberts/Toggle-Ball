#include "GameObstacle.h"

#include "GameBall.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"


void GameObstacle::stepKinematicState(const Smoother& smoother) {
	descriptor->motion->stepKinematicState(kinematicState, smoother);
}


bool GameObstacle::collideWithCap(GameBall& ball, glm::vec3 cap) const {
	glm::vec3 capPosition = kinematicState.getPosition() + kinematicState.getRotation() * cap;
	glm::vec3 capToBall = ball.getKinematicState()->position - capPosition;
	float distanceToBallSq = length2(capToBall);

	float radii = ball.getProperties()->radius + descriptor->shape->getMinorRadius();
	if (distanceToBallSq < radii * radii && distanceToBallSq > 0.000001f) {
		float distanceToBall = std::sqrt(distanceToBallSq);
		ball.collideWithPointOnObstacle(*this, capToBall / distanceToBall, distanceToBall - descriptor->shape->getMinorRadius());
		return true;
	}
	return false;
}

bool GameObstacle::collideWithMidsection(GameBall& ball) const {
	BallCollisionInfo collision = descriptor->shape->getMidsectionCollision(kinematicState, ball);
	if (collision.colliding)
		ball.collideWithPointOnObstacle(*this, collision.normal, collision.separation);
	return collision.colliding;
}

constexpr float SUCCEED_TIME = 0.5f;
bool GameObstacle::notifyOfContactWithBall(const GameBall& ball) {
	goalContactTimer += PHYSICS_TIMESTEP;
	return goalContactTimer >= SUCCEED_TIME && length2(ball.getKinematicState()->velocity) < 0.000001f;
}


PlaneDescriptor GameObstacle::getCapDividingPlane(glm::vec3 cap, float capAngle) const {
	float angle = kinematicState.getAngle() + capAngle;
	glm::vec3 normal = { 0.f, std::cos(angle), std::sin(angle) };
	return { normal, kinematicState.getPosition() + kinematicState.getRotation() * cap };
}