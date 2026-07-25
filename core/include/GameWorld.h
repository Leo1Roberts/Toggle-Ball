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


	glm::vec3 viewOrigin{0.f};
	float heading = 0.f, pitch = 0.f;
	glm::vec3 viewDirection{};
	float viewDistance{};
	glm::vec3 viewPosition{};

	glm::vec3 viewUpDirection{0.f};
	glm::vec3 viewSunDirection{0.f};

	float halfWidth{}, halfHeight{};

	glm::mat4 worldMatrix{}, viewMatrix{}, projectionMatrix{};
	glm::mat3 viewRotationMatrix{};
};


#endif // GAME_WORLD_H
