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
	kinematicState.velocity = kinematicState.angularVelocity = kinematicState.force = kinematicState.torque = 0;
}



void GameBall::addNaturalForces() {
	kinematicState.force += { 0, 0, GRAVITY * properties->mass };
	kinematicState.force -= 0.5f * AIR_DENSITY * properties->dragCoefficient * PI * properties->radius * properties->radius * kinematicState.velocity.length() * kinematicState.velocity;
}

void GameBall::applyForces() {
	if (kinematicState.force.lengthSq() > 0.00000001f) {
		vec3 acc = kinematicState.force / properties->mass;
		kinematicState.velocity += acc * PHYSICS_TIMESTEP;
	}

	if (kinematicState.torque.lengthSq() > 0.00000001f) {
		vec3 angularAcceleration = kinematicState.torque / properties->momentOfInertia;
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
	
	kinematicState.resetForces();
}