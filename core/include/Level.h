#ifndef LEVEL_H
#define LEVEL_H

#include "AssetManager.h"
#include "Obstacle.h"
#include "Ball.h"

class LevelDescriptor {
public:
	LevelDescriptor() :
	    arenaWidth(20),
	    arenaHeight(20),
	    transitionTime(1.0f),
		ballDescriptor(std::make_unique<BallDescriptor>(BASKETBALL, vec2(arenaWidth / 2.f, arenaHeight / 2.f))) {}

	LevelDescriptor(const LevelDescriptor& other);
	LevelDescriptor& operator=(const LevelDescriptor& other);

	LevelDescriptor(const std::string& data);

	static std::unique_ptr<LevelDescriptor> load(const std::string& name) {
		return std::make_unique<LevelDescriptor>(AssetManager::loadTextFile("levels/" + name + ".lvl"));
	}

	void scale();

	[[nodiscard]] std::string serialize() const;

	void setName(const std::string& n) { name = n; }

	[[nodiscard]] float getArenaWidth() const { return arenaWidth; }
	[[nodiscard]] float getArenaHeight() const { return arenaHeight; }
	[[nodiscard]] const std::unique_ptr<BallDescriptor>& getBallDescriptor() const { return ballDescriptor; }
	[[nodiscard]] const std::vector<std::unique_ptr<ObstacleDescriptor>>& getObstacleDescriptors() const { return obstacleDescriptors; }

private:
	std::string name;
	float arenaWidth{};
	float arenaHeight{};
	float transitionTime{};

	std::unique_ptr<BallDescriptor> ballDescriptor;
	std::vector<std::unique_ptr<ObstacleDescriptor>> obstacleDescriptors;
};

#endif // LEVEL_H
