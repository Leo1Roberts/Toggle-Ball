#ifndef LEVEL_H
#define LEVEL_H

#include "utilities/AssetManager.h"
#include "ball/BallDescriptor.h"
#include "obstacle/ObstacleDescriptor.h"


struct LevelDescriptor {
	LevelDescriptor() :
	    arenaWidth(20),
	    arenaHeight(20),
	    transitionTime(1.0f),
		ballDescriptor(std::make_unique<BallDescriptor>(BASKETBALL, glm::vec2(arenaWidth / 2.f, arenaHeight / 2.f))) {}

	LevelDescriptor(const LevelDescriptor& other);
	LevelDescriptor& operator=(const LevelDescriptor& other);

	explicit LevelDescriptor(const std::string& data);

	bool operator==(const LevelDescriptor& other) const;

	static std::unique_ptr<LevelDescriptor> load(const std::string& name) {
		auto level = std::make_unique<LevelDescriptor>(AssetManager::loadTextFile("levels/" + name + ".lvl"));
		level->setName(name);
		return level;
	}

	void scale();

	[[nodiscard]] std::string serialize() const;

	void setName(const std::string& n) { name = n; }

	std::string name;
	float arenaWidth{};
	float arenaHeight{};
	float transitionTime{};

	std::unique_ptr<BallDescriptor> ballDescriptor;
	std::vector<std::unique_ptr<ObstacleDescriptor>> obstacleDescriptors;
};

#endif // LEVEL_H
