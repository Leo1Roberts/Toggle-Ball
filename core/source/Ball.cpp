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