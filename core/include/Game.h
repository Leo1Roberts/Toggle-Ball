#ifndef GAME_H
#define GAME_H

#include "Level.h"
#include "Obstacle.h"
#include "Screen.h"


class Game : public Screen {
public:
	Game(int width, int height) : Screen(width, height) {}

	void play(const LevelDescriptor* levelToPlay);

private:
	void load(const LevelDescriptor* levelToLoad);
	void start();

	bool doProcessEvent(const Event&) override;

	void doUpdate(float dt) override;

	void drawObject(const Mesh<ObjectVertex>* model, const Texture* texture, const vec3& pos, const mat3& rot, const vec3& scale = 1);
	void doDraw() override;

	void resizeLevel();
	void doResize(int, int) override;

	LevelDescriptor level;
	GameBall ball{};
	std::vector<GameObstacle> obstacles;

	Smoother togglePosition{};


	vec3 viewOrigin;
	float heading = 0, pitch = 0;
	vec3 viewDirection;
	float viewDistance{};
	vec3 viewPosition;
	void updateView();

	vec3 viewUpDirection;
	vec3 viewSunDirection;

	float halfHeight{};

	mat4 worldMatrix{}, viewMatrix{}, projectionMatrix{};
	mat3 viewRotationMatrix;
};


#endif // GAME_H