#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "Event.h"
#include "Level.h"


class GameWorld {
public:
	explicit GameWorld(const LevelDescriptor& levelDescriptor);

	void start();

	bool processEvent(const Event& event);
	void update(microseconds dt);
	void render();
	void resize(int screenWidth, int screenHeight);

	[[nodiscard]] bool levelIsComplete() const { return levelComplete; }

private:
	void toggle();

	void updatePhysics(microseconds dt);

	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 position, const glm::mat3& rotation, glm::vec3 scale = glm::vec3(1.f));

	LevelDescriptor level;
	PlaneDescriptor arenaBounds[4];
	GameBall ball{};
	std::vector<GameObstacle> obstacles;

	bool toggled{false};
	Smoother togglePosition{};

	bool levelComplete = false;

	float accumulator{};

	glm::vec3 viewOrigin{};
	float clippingDistance{};
	float halfWidth{}, halfHeight{};
	glm::mat4 worldMatrix{}, viewMatrix{}, projectionMatrix{};

	void resetView();
};


#endif // GAME_WORLD_H
