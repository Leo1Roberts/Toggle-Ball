#ifndef LEVEL_H
#define LEVEL_H

#include "Ball.h"

class LevelDescriptor {
public:
	LevelDescriptor() :
	    name(""),
	    arenaWidth(20),
	    arenaHeight(20),
	    ballType(BASKETBALL),
	    ballPos(0, arenaWidth / 2, arenaHeight / 2),
	    transitionTime(1.0f) {}

	LevelDescriptor(const std::string& data);

	std::string serialize();

	void setName(const std::string& n) { name = n; }

private:
	std::string name;
	float arenaWidth;
	float arenaHeight;
	byte ballType;
	vec3 ballPos;
	float transitionTime;
	std::vector<std::unique_ptr<ObstacleDescriptor>> obstacleDescriptors;
};

#endif // LEVEL_H
