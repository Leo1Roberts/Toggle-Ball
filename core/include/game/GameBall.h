#ifndef GAME_BALL_H
#define GAME_BALL_H

#include "ball/BallDescriptor.h"
#include "ball/BallKinematicState.h"

struct PlaneDescriptor;
class GameObstacle;


class GameBall {
public:
	GameBall() = default;

	explicit GameBall(const BallDescriptor* descriptor) :
		descriptor(descriptor),
		properties(&ballProperties[descriptor->type]),
		texture(getBallTexture(descriptor->type)) {}

	void reset() { descriptor->initKinematicState(kinematicState); }
	void addNaturalForces();
	void applyForces();

	[[nodiscard]] float springForce(float compression) const {
		constexpr float power = 4.f;            // Quartic curve...
		constexpr float forceMultiplier = 10.f; // ...which ends up with a 10x higher force at c = r
		return compression * properties->springConstant + powf(compression, power) * (forceMultiplier - 1.f) * properties->springConstant * powf(properties->radius, 1.f - power);
	}

	void collideWithPlane(const PlaneDescriptor& plane);
	bool collideWithObstacle(GameObstacle& obstacle);
	// normal & separation are polar coordinates for the point on the obstacle (centred on the obstacle, in world space)
	void collideWithPointOnObstacle(const GameObstacle& obstacle, glm::vec3 normal, float separation);

	[[nodiscard]] const Texture* getTexture() const { return texture; }
	[[nodiscard]] const BallKinematicState* getKinematicState() const { return &kinematicState; }
	[[nodiscard]] const BallProperties* getProperties() const { return properties; }

private:
	const BallDescriptor* descriptor{};

	const BallProperties* properties{};
	Texture* texture{};

	BallKinematicState kinematicState;
	BallForces forces;
};


#endif // GAME_BALL_H
