#ifndef GAME_H
#define GAME_H

#include "Level.h"
#include "Obstacle.h"
#include "Plane.h"
#include "Screen.h"
#include "UIManager.h"

#include <glm/glm.hpp>


class Game : public Screen {
public:
	Game(int width, int height);

	void play(const LevelDescriptor* levelToPlay);

private:
	void load(const LevelDescriptor* levelToLoad);
	void start();

	void toggle();

	void doProcessEvent(const Event& event) override;

	void doUpdate(microseconds dt) override;
	void updatePhysics(microseconds dt);

	void doDraw() override;
	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, glm::vec3 pos, const glm::mat3& rot, glm::vec3 scale = glm::vec3(1));

	void doResize(int width, int height) override;
	void resizeLevel();


	UIManager uiManager{};


	LevelDescriptor level;
	PlaneDescriptor arenaBounds[4];
	GameBall ball{};
	std::vector<GameObstacle> obstacles;

	bool toggled{false};
	Smoother togglePosition{};

	float accumulator{};


	glm::vec3 viewOrigin{0.f};
	float heading = 0, pitch = 0;
	glm::vec3 viewDirection{};
	float viewDistance{};
	glm::vec3 viewPosition{};
	void updateView();

	glm::vec3 viewUpDirection{0.f};
	glm::vec3 viewSunDirection{0.f};

	float halfHeight{};

	glm::mat4 worldMatrix{}, viewMatrix{}, projectionMatrix{};
	glm::mat3 viewRotationMatrix{};
};


#endif // GAME_H