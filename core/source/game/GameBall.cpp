#include "game/GameBall.h"

#include "game/GameObstacle.h"
#include "game/Plane.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


void GameBall::addNaturalForces() {
	forces.force += glm::vec3(0.f, 0.f, GRAVITY * properties.mass);
	forces.force -= 0.5f * AIR_DENSITY * properties.dragCoefficient * glm::pi<float>() * properties.radius * properties.radius * length(kinematicState.velocity) * kinematicState.velocity;
}

void GameBall::applyForces() {
	if (length2(forces.force) > 0.00000001f) {
		glm::vec3 acc = forces.force / properties.mass;
		kinematicState.velocity += acc * PHYSICS_TIMESTEP;
	}

	if (length2(forces.torque) > 0.00000001f) {
		glm::vec3 angularAcceleration = forces.torque / properties.momentOfInertia;
		kinematicState.angularVelocity += angularAcceleration * PHYSICS_TIMESTEP;
	}

	kinematicState.position += kinematicState.velocity * PHYSICS_TIMESTEP;
	float t = length(kinematicState.angularVelocity);
	if (t > 0.f) {
		glm::mat3 deltaRot = glm::mat3_cast(glm::angleAxis(t * PHYSICS_TIMESTEP, kinematicState.angularVelocity / t));
		kinematicState.rotation = deltaRot * kinematicState.rotation;
	}

	forces.reset();
}


void GameBall::collideWithPlane(const PlaneDescriptor& plane) { // SIMILAR TO collideWithPointOnObstacle
	float separation = dot(plane.normal, kinematicState.position) - plane.dotProduct;
	if (separation < properties.radius) {
		glm::vec3 planeToPoint = plane.normal * -separation;
		glm::vec3 pointVelocity = kinematicState.velocity + cross(kinematicState.angularVelocity, planeToPoint); // Velocity of the contact point relative to the plane
		float pointPerpendicularSpeed = dot(plane.normal, pointVelocity);

		// Assumes plane is rock solid
		float springFactor = springForce(properties.radius - separation);
		float dampingFactor = properties.dampingCoefficient * pointPerpendicularSpeed;

		float totalForce = springFactor - dampingFactor;
		glm::vec3 force = plane.normal * totalForce;
		forces.force += force;

		glm::vec3 pointParallelVelocity = pointVelocity - plane.normal * pointPerpendicularSpeed;
		float pointParallelSpeedSq = length2(pointParallelVelocity);
		if (pointParallelSpeedSq > 0.00000001f) {
			glm::vec3 friction = pointParallelVelocity * (FRICTION_COEFFICIENTS[properties.material][MAT_CONCRETE] * totalForce / -std::sqrt(pointParallelSpeedSq));
			glm::vec3 maxFriction = pointParallelVelocity * -(properties.mass * properties.momentOfInertia / (PHYSICS_TIMESTEP * (properties.momentOfInertia + properties.mass * separation * separation)));
			if (length2(friction) > length2(maxFriction))
				friction = maxFriction;

			forces.force += friction;
			forces.torque += cross(planeToPoint, friction);
		} else {
			// Spin friction (none in 2D)
			// glm::vec3 spin = plane.normal * dot(plane.normal, kinematicState.angularVelocity);
			// spin.unit();
			// kinematicState.torque -= spin * FRICTION_COEFFICIENTS[properties.material][MAT_CONCRETE] * totalForce * std::cos(std::asin(separation / properties.radius)) * properties.radius * 0.5f;
			float speedSq = length2(kinematicState.velocity);
			if (speedSq > 0.0000001f) {
				glm::vec3 rollingResistance = kinematicState.velocity * (length(force) * ROLLING_RESISTANCE_COEFFICIENTS[properties.material][MAT_CONCRETE] / -std::sqrt(speedSq));
				glm::vec3 maxRollingResistance = kinematicState.velocity * -(properties.mass / PHYSICS_TIMESTEP);
				if (length2(rollingResistance) > length2(maxRollingResistance))
					rollingResistance = maxRollingResistance;

				forces.force += rollingResistance;
			}
		}
	}
}

bool GameBall::collideWithObstacle(GameObstacle& obstacle) {
	bool colliding = false;

	float outerSeparationSq = length2(kinematicState.position - obstacle.getKinematicState()->getPosition());
	float outerRadii = properties.radius + obstacle.getDescriptor()->shape->getBoundingRadius();

	if (outerSeparationSq < outerRadii * outerRadii) { // Within bounding circle
		PlaneDescriptor leftPlane = obstacle.getLeftCapDividingPlane();
		PlaneDescriptor rightPlane = obstacle.getRightCapDividingPlane();
		float leftPlaneDistance = dot(leftPlane.normal, kinematicState.position) - leftPlane.dotProduct;
		float rightPlaneDistance = dot(rightPlane.normal, kinematicState.position) - rightPlane.dotProduct;

		if (obstacle.getDescriptor()->shape->pointIsBetweenCaps(leftPlaneDistance, rightPlaneDistance))
			colliding |= obstacle.collideWithMidsection(*this);
		else {
			if (leftPlaneDistance > 0)
				colliding |= obstacle.collideWithLeftCap(*this);
			if (rightPlaneDistance > 0)
				colliding |= obstacle.collideWithRightCap(*this);
		}
	}
	return colliding && obstacle.notifyOfContactWithBall(*this);
}

void GameBall::collideWithPointOnObstacle(const GameObstacle& obstacle, glm::vec3 normal, float separation) { // SIMILAR TO collideWithPlane
	glm::vec3 ballToPoint = normal * -separation;
	glm::vec3 obstacleToPoint = kinematicState.position - obstacle.getKinematicState()->getPosition() + ballToPoint;
	glm::vec3 ballPointVelocity = kinematicState.velocity + cross(kinematicState.angularVelocity, ballToPoint);
	glm::vec3 obstaclePointVelocity = obstacle.getKinematicState()->getVelocity() + cross({obstacle.getKinematicState()->getAngularSpeed(), 0, 0}, obstacleToPoint);
	glm::vec3 pointVelocity = ballPointVelocity - obstaclePointVelocity; // Velocity of the contact point relative to the obstacle
	float pointPerpendicularSpeed = dot(normal, pointVelocity);

	// Assumes obstacle is rock solid
	float springFactor = springForce(properties.radius - separation);
	float dampingFactor = properties.dampingCoefficient * pointPerpendicularSpeed;

	float totalForce = springFactor - dampingFactor;
	glm::vec3 force = normal * totalForce;
	forces.force += force;

	glm::vec3 pointParallelVelocity = pointVelocity - normal * pointPerpendicularSpeed;
	float pointParallelSpeedSq = length2(pointParallelVelocity);
	if (pointParallelSpeedSq > 0.00000001f) { // Apply friction if sliding
		glm::vec3 friction = pointParallelVelocity * -(FRICTION_COEFFICIENTS[properties.material][obstacle.getDescriptor()->material] * totalForce / std::sqrt(pointParallelSpeedSq));
		glm::vec3 maxFriction = pointParallelVelocity * -(properties.mass * properties.momentOfInertia / (PHYSICS_TIMESTEP * (properties.momentOfInertia + properties.mass * separation * separation)));
		if (length2(friction) > length2(maxFriction))
			friction = maxFriction;

		forces.force += friction;
		forces.torque += cross(ballToPoint, friction);
	}
	if (pointParallelSpeedSq <= 0.0001f) { // Apply rolling resistance if rolling
		glm::vec3 relativeBallVelocity = kinematicState.velocity - obstaclePointVelocity;
		glm::vec3 relativeBallParallelVelocity = relativeBallVelocity - normal * dot(normal, relativeBallVelocity);
		float relativeBallParallelSpeedSq = length2(relativeBallParallelVelocity);
		if (relativeBallParallelSpeedSq > 0.00000001f) {
			glm::vec3 rollingResistance = relativeBallParallelVelocity * -(length(force) * ROLLING_RESISTANCE_COEFFICIENTS[properties.material][obstacle.getDescriptor()->material] / std::sqrt(relativeBallParallelSpeedSq));
			glm::vec3 maxRollingResistance = relativeBallParallelVelocity * -(properties.mass / PHYSICS_TIMESTEP);
			if (length2(rollingResistance) > length2(maxRollingResistance))
				rollingResistance = maxRollingResistance;

			forces.force += rollingResistance;
		}
	}
}