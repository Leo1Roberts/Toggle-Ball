#include "main.h"
#include "Ball.h"

#include <sstream>

BallDescriptor::BallDescriptor(const std::string& data, float version) {
	std::istringstream ss(data);

	std::string ballTypeString;
	char c;
	if (version == 0.1f && !(ss >> ballTypeString >> c >> initialPosition.y >> c >> initialPosition.z >> c) ||
	    version == 0.2f && !(ss >> ballTypeString >> initialPosition.y >> c >> initialPosition.z))
		throw std::invalid_argument("Invalid ball data format");

	for (type = 0; type < static_cast<byte>(std::size(ballString)); type++)
		if (ballTypeString == ballString[type]) break;
	if (type == std::size(ballString))
		throw std::invalid_argument("Unrecognised ball type");
}

std::string BallDescriptor::serialize() const {
	std::ostringstream ss;

	ss << ballString[type] << " " << initialPosition.y << "," << initialPosition.z;

	return ss.str();
}


void BallDescriptor::initKinematicState(BallKinematicState& kinematicState) const {
	kinematicState.position = getInitialPosition();
	kinematicState.rotation = mat3::I;
	kinematicState.velocity = kinematicState.angularVelocity = vec3(0);
}



void GameBall::addNaturalForces() {
	forces.force += { 0, 0, GRAVITY * properties->mass };
	forces.force -= 0.5f * AIR_DENSITY * properties->dragCoefficient * PI * properties->radius * properties->radius * kinematicState.velocity.length() * kinematicState.velocity;
}

void GameBall::applyForces() {
	if (forces.force.lengthSq() > 0.00000001f) {
		vec3 acc = forces.force / properties->mass;
		kinematicState.velocity += acc * PHYSICS_TIMESTEP;
	}

	if (forces.torque.lengthSq() > 0.00000001f) {
		vec3 angularAcceleration = forces.torque / properties->momentOfInertia;
		kinematicState.angularVelocity += angularAcceleration * PHYSICS_TIMESTEP;
	}

	kinematicState.position += kinematicState.velocity * PHYSICS_TIMESTEP;
	float t = kinematicState.angularVelocity.length();
	if (t > 0) {
		mat3 deltaRot;
		kinematicState.angularVelocity /= t;
		deltaRot.R_VecAndAngle(kinematicState.angularVelocity, t * PHYSICS_TIMESTEP);
		kinematicState.angularVelocity *= t;
		kinematicState.rotation = deltaRot * kinematicState.rotation;
	}
	
	forces.reset();
}


void GameBall::collideWithPlane(const PlaneDescriptor& plane) { // SIMILAR TO collideWithPointOnObstacle
	float separation = dot(plane.normal, kinematicState.position) - plane.dotProduct;
	if (separation < properties->radius) {
		vec3 planeToPoint = plane.normal * -separation;
		vec3 pointVelocity = kinematicState.velocity + cross(kinematicState.angularVelocity, planeToPoint); // Velocity of the contact point relative to the plane
		float pointPerpendicularSpeed = dot(plane.normal, pointVelocity);

		// Assumes plane is rock solid
		float springFactor = springForce(properties->radius - separation);
		float dampingFactor = properties->dampingCoefficient * pointPerpendicularSpeed;

		float totalForce = springFactor - dampingFactor;
		vec3 force = plane.normal * totalForce;
		forces.force += force;

		vec3 pointParallelVelocity = pointVelocity - plane.normal * pointPerpendicularSpeed;
		float pointParallelSpeedSq = pointParallelVelocity.lengthSq();
		if (pointParallelSpeedSq > 0.00000001f) {
			vec3 friction = pointParallelVelocity / -std::sqrt(pointParallelSpeedSq) * FRICTION_COEFFICIENTS[properties->material][MAT_CONCRETE] * totalForce;
			vec3 maxFriction = pointParallelVelocity * -properties->mass * properties->momentOfInertia / (PHYSICS_TIMESTEP * (properties->momentOfInertia + properties->mass * separation * separation));
			if (friction.lengthSq() > maxFriction.lengthSq())
				friction = maxFriction;

			forces.force += friction;
			forces.torque += cross(planeToPoint, friction);
		} else {
			// Spin friction (none in 2D)
			// vec3 spin = plane.normal * dot(plane.normal, kinematicState.angularVelocity);
			// spin.unit();
			// kinematicState.torque -= spin * FRICTION_COEFFICIENTS[properties->material][MAT_CONCRETE] * totalForce * std::cos(std::asin(separation / properties->radius)) * properties->radius * 0.5f;
			float speedSq = kinematicState.velocity.lengthSq();
			if (speedSq > 0.0000001f) {
				vec3 rollingResistance = kinematicState.velocity / -std::sqrt(speedSq) * force.length() * ROLLING_RESISTANCE_COEFFICIENTS[properties->material][MAT_CONCRETE];
				vec3 maxRollingResistance = kinematicState.velocity * -properties->mass / PHYSICS_TIMESTEP;
				if (rollingResistance.lengthSq() > maxRollingResistance.lengthSq())
					rollingResistance = maxRollingResistance;

				forces.force += rollingResistance;
			}
		}
	}
}

bool GameBall::collideWithObstacle(GameObstacle& obstacle) {
	bool colliding = false;

	float outerSeparationSq = (kinematicState.position - obstacle.getKinematicState()->getPosition()).lengthSq();
	float outerRadii = properties->radius + obstacle.getDescriptor()->getShape()->getBoundingRadius();

	if (outerSeparationSq < outerRadii * outerRadii) { // Within bounding circle
		PlaneDescriptor leftPlane = obstacle.getLeftCapDividingPlane();
		PlaneDescriptor rightPlane = obstacle.getRightCapDividingPlane();
		float leftPlaneDistance = dot(leftPlane.normal, kinematicState.position) - leftPlane.dotProduct;
		float rightPlaneDistance = dot(rightPlane.normal, kinematicState.position) - rightPlane.dotProduct;

		if (obstacle.getDescriptor()->getShape()->pointIsBetweenCaps(leftPlaneDistance, rightPlaneDistance))
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

void GameBall::collideWithPointOnObstacle(const GameObstacle& obstacle, vec3 normal, float separation) { // SIMILAR TO collideWithPlane
	vec3 ballToPoint = normal * -separation;
	vec3 obstacleToPoint = kinematicState.position - obstacle.getKinematicState()->getPosition() + ballToPoint;
	vec3 ballPointVelocity = kinematicState.velocity + cross(kinematicState.angularVelocity, ballToPoint);
	vec3 obstaclePointVelocity = obstacle.getKinematicState()->getVelocity() + cross({obstacle.getKinematicState()->getAngularVelocity(), 0, 0}, obstacleToPoint);
	vec3 pointVelocity = ballPointVelocity - obstaclePointVelocity; // Velocity of the contact point relative to the obstacle
	float pointPerpendicularSpeed = dot(normal, pointVelocity);

	// Assumes obstacle is rock solid
	float springFactor = springForce(properties->radius - separation);
	float dampingFactor = properties->dampingCoefficient * pointPerpendicularSpeed;

	float totalForce = springFactor - dampingFactor;
	vec3 force = normal * totalForce;
	forces.force += force;

	vec3 pointParallelVelocity = pointVelocity - normal * pointPerpendicularSpeed;
	float pointParallelSpeedSq = pointParallelVelocity.lengthSq();
	if (pointParallelSpeedSq > 0.00000001f) { // Apply friction if sliding
		vec3 friction = pointParallelVelocity / -std::sqrt(pointParallelSpeedSq) * FRICTION_COEFFICIENTS[properties->material][obstacle.getDescriptor()->getMaterial()] * totalForce;
		vec3 maxFriction = pointParallelVelocity * -properties->mass * properties->momentOfInertia / (PHYSICS_TIMESTEP * (properties->momentOfInertia + properties->mass * separation * separation));
		if (friction.lengthSq() > maxFriction.lengthSq())
			friction = maxFriction;

		forces.force += friction;
		forces.torque += cross(ballToPoint, friction);
	}
	if (pointParallelSpeedSq <= 0.0001f) { // Apply rolling resistance if rolling
		vec3 relativeBallVelocity = kinematicState.velocity - obstaclePointVelocity;
		vec3 relativeBallParallelVelocity = relativeBallVelocity - normal * dot(normal, relativeBallVelocity);
		float relativeBallParallelSpeedSq = relativeBallParallelVelocity.lengthSq();
		if (relativeBallParallelSpeedSq > 0.00000001f) {
			vec3 rollingResistance = relativeBallParallelVelocity / -std::sqrt(relativeBallParallelSpeedSq) * force.length() * ROLLING_RESISTANCE_COEFFICIENTS[properties->material][obstacle.getDescriptor()->getMaterial()];
			vec3 maxRollingResistance = relativeBallParallelVelocity * -properties->mass / PHYSICS_TIMESTEP;
			if (rollingResistance.lengthSq() > maxRollingResistance.lengthSq())
				rollingResistance = maxRollingResistance;

			forces.force += rollingResistance;
		}
	}
}