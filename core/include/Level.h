#ifndef LEVEL_H
#define LEVEL_H

class LevelDescriptor {
public:
	LevelDescriptor(const std::string& name) :
		name(name),
		arenaWidth(20),
		arenaHeight(20),
		ballType(BASKETBALL),
		ballPos(arenaWidth / 2, arenaHeight / 2),
		transitionTime(1) {}

private:
	std::string name;
	float arenaWidth;
	float arenaHeight;
	byte ballType;
	vec2 ballPos;
	float transitionTime;
	std::vector<std::unique_ptr<ObstacleDescriptor>> obstacleDefinitions;
};

#endif // LEVEL_H
