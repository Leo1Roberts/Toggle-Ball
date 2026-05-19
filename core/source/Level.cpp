#include "main.h"
#include "Obstacle.h"
#include "Level.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

LevelDescriptor::LevelDescriptor(const std::string& levelData) {
	std::istringstream ss(levelData);

	std::string title;
	title.resize(13);
	ss.read(title.data(), 13);
	float version;
	if (title.compare("Toggle Ball v") != 0 || !(ss >> version))
		throw std::invalid_argument("String is not a Toggle Ball level");

	if (version != 0.1f)
		throw std::logic_error("Unsupported level format");

	char c;

	while (true) {
		char header;
		if (!(ss >> header)) break;

		if (header == 'b') {
			std::string ballTypeString;
			if (!(ss >> ballTypeString >> c >> ballPos.y >> c >> ballPos.z >> c))
				throw std::invalid_argument("Invalid ball data format");

			std::transform(ballTypeString.begin(), ballTypeString.end(), ballTypeString.begin(), ::tolower);
			if (ballTypeString == "basketball")
				ballType = BASKETBALL;
			else
				throw std::invalid_argument("Unrecognised ball type");
		} else if (header == 'a') {
			if (!(ss >> c >> arenaWidth >> c >> arenaHeight >> c))
				throw std::invalid_argument("Invalid arena data format");
		} else if (header == 't') {
			if (!(ss >> transitionTime))
				throw std::invalid_argument("Invalid transition time data format");
		} else if (header == 'o') {
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

			std::transform(isStraightString.begin(), isStraightString.end(), isStraightString.begin(), ::tolower);
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

			obstacleDescriptors.push_back(std::make_unique<ObstacleDescriptor>(std::move(motionSpec), std::move(shapeSpec), isGoal));
		}
	}
}