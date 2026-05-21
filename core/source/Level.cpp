#include "main.h"
#include "Obstacle.h"
#include "Level.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

LevelDescriptor::LevelDescriptor(const std::string& data) {
	std::istringstream ss(data);

	std::string title;
	title.resize(13);
	ss.read(title.data(), 13);
	float version;
	if (title.compare("Toggle Ball v") != 0 || !(ss >> version))
		throw std::invalid_argument("String is not a Toggle Ball level");

	char c;

	if (version == 0.2f)
		std::getline(ss >> std::ws, name);

	while (true) {
		char header;
		if (!(ss >> header)) break;

		if (header == 'b') {
			std::string ballTypeString;
			if (version == 0.1f && !(ss >> ballTypeString >> c >> ballPos.y >> c >> ballPos.z >> c) ||
			    version == 0.2f && !(ss >> ballTypeString >> ballPos.y >> c >> ballPos.z))
				throw std::invalid_argument("Invalid ball data format");

			for (ballType = 0; ballType < std::size(ballString); ballType++)
				if (ballTypeString == ballString[ballType]) break;
			if (ballType == std::size(ballString))
				throw std::invalid_argument("Unrecognised ball type");
		} else if (header == 'a') {
			if (version == 0.1f && !(ss >> c >> arenaWidth >> c >> arenaHeight >> c) ||
			    version == 0.2f && !(ss >> arenaWidth >> c >> arenaHeight))
				throw std::invalid_argument("Invalid arena data format");
		} else if (header == 't') {
			if (!(ss >> transitionTime))
				throw std::invalid_argument("Invalid transition time data format");
		} else if (header == 'o') {
			if (version == 0.1f) {
				vec2 pos;
				float angle;
				std::string isStraightString;
				float minorRadius;
				vec2 start;
				vec2 end;
				std::string stateTypeString;
				vec3 stateA;
				vec3 stateB;
				std::string isGoalString;

				if (!(ss >> c >> pos.x >> c >> pos.y >> c >> angle >> isStraightString >> minorRadius >> c >> start.x >> c >> start.y >> c >> c >> end.x >> c >> end.y >> c >> stateTypeString >> c >> stateA.x >> c >> stateA.y >> c >> stateA.z >> c >> c >> stateB.x >> c >> stateB.y >> c >> stateB.z >> c >> isGoalString))
					throw std::invalid_argument("Invalid obstacle data format");

				std::unique_ptr<AbstractShapeSpec> shapeSpec;
				if (isStraightString == "true")
					shapeSpec = std::make_unique<SegmentSpec>(minorRadius, -end.x, start.x);
				else if (isStraightString == "false") {
					float arcRadius = start.length();
					shapeSpec = std::make_unique<ArcSpec>(minorRadius, 2 * acos(start.y / arcRadius), arcRadius);
				} else
					throw std::invalid_argument("Invalid obstacle data format");

				transform(isGoalString.begin(), isGoalString.end(), isGoalString.begin(), ::tolower);
				bool isGoal;
				if (isGoalString == "true")
					isGoal = true;
				else if (isGoalString == "false")
					isGoal = false;
				else
					throw std::invalid_argument("Invalid obstacle data format");

				transform(stateTypeString.begin(), stateTypeString.end(), stateTypeString.begin(), ::tolower);
				std::unique_ptr<IMotionSpec> motionSpec;
				if (stateTypeString == "static")
					motionSpec = std::make_unique<StaticSpec>(pos, angle);
				else if (stateTypeString == "pos")
					motionSpec = std::make_unique<TogglingPositionSpec>(angle, vec2(stateA.y, stateA.z), vec2(stateB.y, stateB.z));
				else if (stateTypeString == "pos_osc")
					motionSpec = std::make_unique<OscillatingPositionSpec>(angle, vec2(stateA.y, stateA.z), vec2(stateB.y, stateB.z), stateA.x, stateB.x);
				else if (stateTypeString == "ang")
					motionSpec = std::make_unique<TogglingAngleSpec>(pos, stateA.x, stateB.x);
				else if (stateTypeString == "ang_vel")
					motionSpec = std::make_unique<SpinningSpec>(pos, angle, stateA.x, stateB.x);
				else if (stateTypeString == "ang_osc")
					motionSpec = std::make_unique<OscillatingAngleSpec>(pos, stateA.y, stateB.y, stateA.x, stateB.x);
				else
					throw std::invalid_argument("Unrecognised obstacle state type");

				obstacleDescriptors.push_back(std::make_unique<ObstacleDescriptor>(std::move(shapeSpec), std::move(motionSpec), isGoal));
			} else if (version == 0.2f) {
				std::string obstacleData;
				std::getline(ss, obstacleData);
				obstacleDescriptors.push_back(std::make_unique<ObstacleDescriptor>(obstacleData));
			} else
				throw std::logic_error("Unsupported file version");
		}
	}
}


std::string LevelDescriptor::serialize() {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss
	<< "Toggle Ball v0.2\n"
	<< name
	<< "\nb " << ballString[ballType] << " " << ballPos.y << "," << ballPos.z
	<< "\na " << arenaWidth << "," << arenaHeight
	<< "\nt " << transitionTime;

	for (auto& d: obstacleDescriptors)
		ss << "\no " << d->serialize();

	return ss.str();
}
