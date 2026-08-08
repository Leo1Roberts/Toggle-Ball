#include "ball/BallDescriptor.h"

#include "ball/BallKinematicState.h"

#include <sstream>


BallDescriptor::BallDescriptor(const std::string& data, float version) {
	std::istringstream ss(data);

	std::string ballTypeString;
	char c;
	if (version == 0.1f && !(ss >> ballTypeString >> c >> initialPosition.x >> c >> initialPosition.y >> c) ||
		version == 0.2f && !(ss >> ballTypeString >> initialPosition.x >> c >> initialPosition.y))
		throw std::invalid_argument("Invalid ball data format");

	for (int t = 0; t < (int)BallType::COUNT; t++)
		if (ballTypeString == getBallString((BallType)t)) {
			type = (BallType)t;
			break;
		}
	if (type == BallType::COUNT)
		throw std::invalid_argument("Unrecognised ball type");
}

std::string BallDescriptor::serialize() const {
	std::ostringstream ss;

	ss << getBallString(type) << " " << initialPosition.x << "," << initialPosition.y;

	return ss.str();
}


void BallDescriptor::initKinematicState(BallKinematicState& kinematicState) const {
	kinematicState.position = planarToWorld(initialPosition);
	kinematicState.rotation = glm::mat3(1);
	kinematicState.velocity = kinematicState.angularVelocity = glm::vec3(0.f);
}