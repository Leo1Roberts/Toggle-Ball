#include "obstacle/ObstacleDescriptor.h"

#include <sstream>


ObstacleDescriptor::ObstacleDescriptor(const ObstacleDescriptor& other) :
	goal(other.goal), color(other.color), material(other.material) {
	if (other.shape)
		shape = other.shape->clone();
	if (other.motion)
		motion = other.motion->clone();
}

ObstacleDescriptor& ObstacleDescriptor::operator=(const ObstacleDescriptor& other) {
	if (this != &other) {
		goal = other.goal;
		color = other.color;
		material = other.material;
		if (other.shape)
			shape = other.shape->clone();
		if (other.motion)
			motion = other.motion->clone();
	}
	return *this;
}


ObstacleDescriptor::ObstacleDescriptor(const std::string& data) {
	std::istringstream ss(data);

	std::string shapeString, motionString;

	if (!(std::getline(ss, shapeString, '|') && std::getline(ss, motionString, '|') && ss >> goal))
		throw std::invalid_argument("Invalid obstacle data format");

	shape = std::move(AbstractShapeSpec::deserialize(shapeString));
	motion = std::move(IMotionSpec::deserialize(motionString));
	material = MAT_CONCRETE;
}

std::string ObstacleDescriptor::serialize() const {
	std::ostringstream ss;

	ss << shape->serialize() << "|" << motion->serialize() << "|" << goal;

	return ss.str();
}


bool ObstacleDescriptor::operator==(const ObstacleDescriptor& other) const {
	return
	*shape == *other.shape &&
	*motion == *other.motion &&
	goal == other.goal &&
	color == other.color &&
	material == other.material;
}


void ObstacleDescriptor::scale(float factor) {
	shape->scale(factor);
	motion->scale(factor);
}