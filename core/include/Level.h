#ifndef LEVEL_H
#define LEVEL_H

#include "Ball.h"

class LevelDescriptor {
public:
	LevelDescriptor() = default;
	LevelDescriptor(const std::string& levelData);

private:
	std::string name = "";
	float arenaWidth = 20;
	float arenaHeight = 20;
	byte ballType = BASKETBALL;
	vec3 ballPos = vec3(0, arenaWidth / 2, arenaHeight / 2);
	float transitionTime = 1.0f;
	std::vector<std::unique_ptr<ObstacleDescriptor>> obstacleDescriptors;
};

#endif // LEVEL_H
