#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "Camera.h"
#include "Event.h"
#include "Level.h"


class GameWorld {
public:
	explicit GameWorld(const LevelDescriptor& levelDescriptor);

	void start();

	bool processEvent(const Event& event);
	void update(microseconds dt);
	void render() const;
	void resize(int screenWidth, int screenHeight);

	[[nodiscard]] bool levelIsComplete() const { return levelComplete; }

private:
	void toggle();

	void updatePhysics(microseconds dt);

	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale = glm::vec3(1.f)) const;

	LevelDescriptor level;
	PlaneDescriptor arenaBounds[4];
	GameBall ball{};
	std::vector<GameObstacle> obstacles;

	bool toggled = false;
	Smoother togglePosition{};
	bool levelComplete = false;
	float accumulator{};

	Camera camera;
};


#endif // GAME_WORLD_H
