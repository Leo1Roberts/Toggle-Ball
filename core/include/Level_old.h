#ifndef LEVEL_OLD_H
#define LEVEL_OLD_H

#include "Object.h"

constexpr int MAX_OBSTACLES = 200;

struct Level {
	std::string name;
	byte ballType;

	vec3 ballPos;
	float arenaWidth;
	float arenaHeight;
	float transitionTime;
	std::vector<ObstacleDefinition> obstacleDefinitions;

	Level() : name(""),
	          ballType(0),
	          ballPos(0),
	          arenaWidth(0),
	          arenaHeight(0),
	          transitionTime(0) {}

	bool operator==(const Level& other) const;

	std::string exportLevel();

	static bool importLevel(Level* level, const std::string& levelData, const std::string& levelName);
};

#endif // LEVEL_OLD_H
