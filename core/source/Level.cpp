#include "main.h"
#include "Colors.h"
#include "LoadOBJ.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "Level.h"

bool Level::operator==(const Level& other) const {
	return
	name == other.name &&
	ballType == other.ballType &&
	ballPos == other.ballPos &&
	arenaWidth == other.arenaWidth &&
	arenaHeight == other.arenaHeight &&
	transitionTime == other.transitionTime &&
	obstacleDefinitions == other.obstacleDefinitions;
}

std::string Level::exportLevel() {
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(6);

	ss << "Toggle Ball v0.1\n";

	std::string ballTypeString;
	switch (ballType) {
	case BASKETBALL:
		ballTypeString = "Basketball";
		break;
	default:
		ballTypeString = "[Unknown]";
	}

	ss
	<< "\nb " << ballTypeString
	<< " {" << ballPos.y << "," << ballPos.z << "}"
	<< "\na {" << arenaWidth << "," << arenaHeight << "}"
	<< "\nt " << transitionTime << "\n";

	for (ObstacleDefinition& d: obstacleDefinitions) {
		ss
		<< "\no {" << d.initPos.y << "," << d.initPos.z << "} " << d.initAngle
		<< (d.isStraight ? " true " : " false ")
		<< d.minorRadius
		<< " {" << d.start.y << "," << d.start.z << "} {"
		<< d.end.y << "," << d.end.z << "} ";

		std::string st;
		switch (d.stateType) {
		case ST_STATIC:
			st = "static {";
			break;
		case ST_POS:
			st = "pos {";
			break;
		case ST_POS_OSC:
			st = "pos_osc {";
			break;
		case ST_ANG:
			st = "ang {";
			break;
		case ST_ANG_VEL:
			st = "ang_vel {";
			break;
		case ST_ANG_OSC:
			st = "ang_osc {";
			break;
		}

		ss
		<< st
		<< d.stateA.x << "," << d.stateA.y << "," << d.stateA.z << "} {"
		<< d.stateB.x << "," << d.stateB.y << "," << d.stateB.z << "} "
		<< (d.isGoal ? "true" : "false");
	}

	return ss.str();
}

bool Level::importLevel(Level* level, const std::string& levelData, const std::string& levelName) {
	std::istringstream ss(levelData);

	char title[14] = {0};
	ss.read(title, 13);
	float version;
	if (strcmp(title, "Toggle Ball v") == 0 && ss >> version) {
		level->name = levelName;
	} else
		return false;

	char c;

	while (true) {
		char header;
		if (!(ss >> header)) break;

		if (header == 'b') {
			char ballTypeString[20];
			if (!(ss >> ballTypeString >> c >> level->ballPos.y >> c >> level->ballPos.z >> c))
				return false;
			level->ballPos.x = 0;
			std::string bt = ballTypeString;
			transform(bt.begin(), bt.end(), bt.begin(), tolower);
			if (strcmp(bt.c_str(), "basketball") == 0)
				level->ballType = BASKETBALL;
			else
				level->ballType = BASKETBALL; // Default
		} else if (header == 'a') {
			if (!(ss >> c >> level->arenaWidth >> c >> level->arenaHeight >> c))
				return false;
		} else if (header == 't') {
			if (!(ss >> level->transitionTime))
				return false;
		} else if (header == 'o' && level->obstacleDefinitions.size() < MAX_OBSTACLES) {
			vec3 pos = {0};
			float angle;
			char isStraightText[6] = {0};
			float minorRadius;
			vec3 start = {0};
			vec3 end = {0};
			char stateTypeText[8] = {0};
			vec3 stateA = {0};
			vec3 stateB = {0};
			char isGoalText[6] = {0};

			if (!(ss >> c >> pos.y >> c >> pos.z >> c >> angle >> isStraightText >> minorRadius >> c >> start.y >> c >> start.z >> c >> c >> end.y >> c >> end.z >> c >> stateTypeText >> c >> stateA.x >> c >> stateA.y >> c >> stateA.z >> c >> c >> stateB.x >> c >> stateB.y >> c >> stateB.z >> c >> isGoalText))
				return false;

			std::string isStraightString = isStraightText;
			transform(isStraightString.begin(), isStraightString.end(), isStraightString.begin(), tolower);
			bool isStraight;
			if (strcmp(isStraightString.c_str(), "true") == 0) isStraight = true;
			else if (strcmp(isStraightString.c_str(), "false") == 0)
				isStraight = false;
			else
				return false;

			std::string stateTypeString = stateTypeText;
			transform(stateTypeString.begin(), stateTypeString.end(), stateTypeString.begin(), tolower);
			byte stateType;
			if (strcmp(stateTypeString.c_str(), "static") == 0) stateType = ST_STATIC;
			else if (strcmp(stateTypeString.c_str(), "pos") == 0)
				stateType = ST_POS;
			else if (strcmp(stateTypeString.c_str(), "pos_osc") == 0)
				stateType = ST_POS_OSC;
			else if (strcmp(stateTypeString.c_str(), "ang") == 0)
				stateType = ST_ANG;
			else if (strcmp(stateTypeString.c_str(), "ang_vel") == 0)
				stateType = ST_ANG_VEL;
			else if (strcmp(stateTypeString.c_str(), "ang_osc") == 0)
				stateType = ST_ANG_OSC;
			else
				return false;

			std::string isGoalString = isGoalText;
			transform(isGoalString.begin(), isGoalString.end(), isGoalString.begin(), tolower);
			bool isGoal;
			if (strcmp(isGoalString.c_str(), "true") == 0) isGoal = true;
			else if (strcmp(isGoalString.c_str(), "false") == 0)
				isGoal = false;
			else
				return false;

			level->obstacleDefinitions.emplace_back(pos, angle, isStraight, start, end, minorRadius, stateType, stateA, stateB, isGoal);
		}
	}

	return true;
}