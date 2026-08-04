#ifndef GAME_OBSTACLE_H
#define GAME_OBSTACLE_H

#include "opengl/Mesh.h"
#include "obstacle/ObstacleDescriptor.h"
#include "obstacle/ObstacleKinematicState.h"
#include "Plane.h"
#include "utilities/Utilities.h"


class GameObstacle {
public:
	explicit GameObstacle(const ObstacleDescriptor* descriptor) :
		descriptor(descriptor) {
		reset();
		descriptor->generateObstacleMesh(mesh);
	}

	~GameObstacle() = default;

	GameObstacle(const GameObstacle& other) = delete;
	GameObstacle& operator=(const GameObstacle&) = delete;
	GameObstacle(GameObstacle&&) = default;
	GameObstacle& operator=(GameObstacle&&) = default;

	void reset() {
		descriptor->motion->initKinematicState(kinematicState);
		goalContactTimer = 0;
	}
	void stepKinematicState(const Smoother& smoother);

	bool collideWithLeftCap(GameBall& ball) const { return collideWithCap(ball, planarToWorld(descriptor->shape->getLeftCap())); }
	bool collideWithRightCap(GameBall& ball) const { return collideWithCap(ball, planarToWorld(descriptor->shape->getRightCap())); }
	bool collideWithMidsection(GameBall& ball) const;
	bool notifyOfContactWithBall(const GameBall& ball);

	[[nodiscard]] const ObstacleDescriptor* getDescriptor() const { return descriptor; }
	[[nodiscard]] const ObstacleKinematicState* getKinematicState() const { return &kinematicState; }
	[[nodiscard]] const Mesh<ObjectVertex>* getMesh() const { return &mesh; }
	[[nodiscard]] PlaneDescriptor getLeftCapDividingPlane() const {
		return getCapDividingPlane(planarToWorld(descriptor->shape->getLeftCap()), descriptor->shape->getLeftCapAngle());
	}
	[[nodiscard]] PlaneDescriptor getRightCapDividingPlane() const {
		return getCapDividingPlane(planarToWorld(descriptor->shape->getRightCap()), descriptor->shape->getRightCapAngle());
	}

private:
	const ObstacleDescriptor* descriptor;

	ObstacleKinematicState kinematicState;
	float goalContactTimer{};

	Mesh<ObjectVertex> mesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);

	bool collideWithCap(GameBall& ball, glm::vec3 cap) const;

	[[nodiscard]] PlaneDescriptor getCapDividingPlane(glm::vec3 cap, float capAngle) const;
};


#endif // GAME_OBSTACLE_H
