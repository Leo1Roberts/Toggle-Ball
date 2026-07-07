#ifndef LEVEL_H
#define LEVEL_H

#include "AssetManager.h"
#include "Ball.h"

class ObstacleDescriptor;

class LevelDescriptor {
public:
	LevelDescriptor() :
	    arenaWidth(20),
	    arenaHeight(20),
	    ballType(BASKETBALL),
	    ballPos(0, arenaWidth / 2, arenaHeight / 2),
	    transitionTime(1.0f) {}

	LevelDescriptor(const LevelDescriptor& other);
	LevelDescriptor& operator=(const LevelDescriptor& other);

	LevelDescriptor(const std::string& data);
	static std::unique_ptr<LevelDescriptor> load(const std::string& name) {
		return std::make_unique<LevelDescriptor>(AssetManager::loadTextFile("levels/" + name + ".lvl"));
	}

	[[nodiscard]] std::string serialize() const;

	void setName(const std::string& n) { name = n; }

	[[nodiscard]] byte getBallType() const { return ballType; }
	[[nodiscard]] const std::vector<std::unique_ptr<ObstacleDescriptor>>& getObstacleDescriptors() { return obstacleDescriptors; }

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
