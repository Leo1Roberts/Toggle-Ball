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


void ObstacleDescriptor::changeMotion(IMotionSpec::Type type, IMotionSpec::IncompletePropertyValues& values, bool toggled) {
	for (const auto& desc : motion->getPropertyDescriptors()) {
		auto value = motion->getProperty(false, desc);
		values[(int)desc.property][(int)desc.associatedState] = value;

		switch (desc.property) {
		case IMotionSpec::Property::Position_X:
			if (desc.associatedState == IMotionSpec::State::A) {
				values[(int)IMotionSpec::Property::Position1_X][(int)IMotionSpec::State::_] = value;
				if (!toggled)
					values[(int)IMotionSpec::Property::Position_X][(int)IMotionSpec::State::_] = value;
			} else if (desc.associatedState == IMotionSpec::State::B) {
				values[(int)IMotionSpec::Property::Position2_X][(int)IMotionSpec::State::_] = value;
				if (toggled)
					values[(int)IMotionSpec::Property::Position_X][(int)IMotionSpec::State::_] = value;
			}
			break;
		case IMotionSpec::Property::Position_Y:
			if (desc.associatedState == IMotionSpec::State::A) {
				values[(int)IMotionSpec::Property::Position1_Y][(int)IMotionSpec::State::_] = value;
				if (!toggled)
					values[(int)IMotionSpec::Property::Position_Y][(int)IMotionSpec::State::_] = value;
			} else if (desc.associatedState == IMotionSpec::State::B) {
				values[(int)IMotionSpec::Property::Position2_Y][(int)IMotionSpec::State::_] = value;
				if (toggled)
					values[(int)IMotionSpec::Property::Position_Y][(int)IMotionSpec::State::_] = value;
			}
			break;
		case IMotionSpec::Property::Position1_X:
			if (!toggled)
				values[(int)IMotionSpec::Property::Position_X][(int)IMotionSpec::State::_] = value;
			values[(int)IMotionSpec::Property::Position_X][(int)IMotionSpec::State::A] = value;
			break;
		case IMotionSpec::Property::Position1_Y:
			if (!toggled)
				values[(int)IMotionSpec::Property::Position_Y][(int)IMotionSpec::State::_] = value;
			values[(int)IMotionSpec::Property::Position_Y][(int)IMotionSpec::State::A] = value;
			break;
		case IMotionSpec::Property::Position2_X:
			if (toggled)
				values[(int)IMotionSpec::Property::Position_X][(int)IMotionSpec::State::_] = value;
			values[(int)IMotionSpec::Property::Position_X][(int)IMotionSpec::State::B] = value;
			break;
		case IMotionSpec::Property::Position2_Y:
			if (toggled)
				values[(int)IMotionSpec::Property::Position_Y][(int)IMotionSpec::State::_] = value;
			values[(int)IMotionSpec::Property::Position_Y][(int)IMotionSpec::State::B] = value;
			break;
		case IMotionSpec::Property::Angle:
			if (desc.associatedState == IMotionSpec::State::A) {
				values[(int)IMotionSpec::Property::Angle1][(int)IMotionSpec::State::_] = value;
				if (!toggled) {
					values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::_] = value;
					values[(int)IMotionSpec::Property::InitialAngle][(int)IMotionSpec::State::_] = value;
				}
			} else if (desc.associatedState == IMotionSpec::State::B) {
				values[(int)IMotionSpec::Property::Angle2][(int)IMotionSpec::State::_] = value;
				if (toggled) {
					values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::_] = value;
					values[(int)IMotionSpec::Property::InitialAngle][(int)IMotionSpec::State::_] = value;
				}
			}
			break;
		case IMotionSpec::Property::InitialAngle:
			values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::_] = value;
			break;
		case IMotionSpec::Property::Angle1:
			if (!toggled)
				values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::_] = value;
			values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::A] = value;
			break;
		case IMotionSpec::Property::Angle2:
			if (toggled)
				values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::_] = value;
			values[(int)IMotionSpec::Property::Angle][(int)IMotionSpec::State::B] = value;
			break;
		default:;
		}
	}

	motion = IMotionSpec::make(type, values, toggled);
	updateColor();
}


void ObstacleDescriptor::setIsGoal(bool isGoal) {
	goal = isGoal;
	updateColor();
}