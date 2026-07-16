#ifndef BALL_H
#define BALL_H

#include "Texture.h"
#include "Mesh.h"
#include "Obstacle.h"
#include "PhysicsConstants.h"
#include "Plane.h"

class GameObstacle;

enum {
	BASKETBALL,
	FOOTBALL,
	PING_PONG,
	MARBLE
};

const std::string ballString[] = {"basketball", "football", "ping-pong", "marble"};

struct BallProperties {
	constexpr BallProperties(
		byte material,
		float radius,
		float mass,
		float momentOfInertia,
		float springConstant,
		float dampingCoefficient,
		float dragCoefficient) :
		material(material),
		radius(radius),
		mass(mass),
		momentOfInertia(momentOfInertia),
		springConstant(springConstant),
		springConstantInv(1.f/springConstant),
		dampingCoefficient(dampingCoefficient),
		dampingCoefficientInv(1.f/dampingCoefficient),
		dragCoefficient(dragCoefficient) {}

	byte material;				// SSOT

	float radius;				// SSOT
	float mass;					// SSOT
	float momentOfInertia;		// SSOT
	float springConstant;		// SSOT
	float springConstantInv;
	float dampingCoefficient;	// SSOT
	float dampingCoefficientInv;
	float dragCoefficient;		// SSOT
};

struct BallKinematicState {
	vec3 position;
	mat3 rotation = mat3::I;
	vec3 velocity;
	vec3 angularVelocity;
};

struct BallForces {
	vec3 force;
	vec3 torque;

	void reset() { force = torque = vec3(0); }
};

struct BallCollisionInfo {
	bool colliding{};
	vec3 normal{};
	float separation{};
};



static constexpr BallProperties ballProperties[] = {
	{
		MAT_BASKETBALL,
		0.12f,
		0.45f,
		0.65f * 0.45f * 0.12f * 0.12f,
		4000.f,
		4.f,
		0.5f
	}
};

static Texture* getBallTexture(byte ballType) {
	switch (ballType) {
	case BASKETBALL:
		return Textures::basketball.get();
	case FOOTBALL:
		[[fallthrough]];
	case PING_PONG:
		[[fallthrough]];
	case MARBLE:
		[[fallthrough]];
	default:
		return Textures::white.get();
	}
}



class BallDescriptor {
public:
	constexpr BallDescriptor(byte type, vec2 initialPosition) :
		type(type),
		initialPosition(planarToWorld(initialPosition)) {}

	BallDescriptor(const std::string& data, float version);
	[[nodiscard]] std::string serialize() const;

	void scale() {
		initialPosition *= getRadius();
	}

	void initKinematicState(BallKinematicState& kinematicState) const;

	[[nodiscard]] byte getType() const { return type; }
	[[nodiscard]] vec3 getInitialPosition() const { return initialPosition; }
	[[nodiscard]] float getRadius() const { return ballProperties[type].radius; }

private:
	byte type;
	vec3 initialPosition;
};

class GameBall {
public:
	GameBall() = default;

	explicit GameBall(const BallDescriptor* descriptor) :
		descriptor(descriptor),
		properties(&ballProperties[descriptor->getType()]),
		texture(getBallTexture(descriptor->getType())) {}

	void reset() { descriptor->initKinematicState(kinematicState); }
	void addNaturalForces();
	void applyForces();

	[[nodiscard]] float springForce(float compression) const {
		constexpr float power = 4;            // Quartic curve...
		constexpr float forceMultiplier = 10; // ...which ends up with a 10x higher force at c = r
		return compression * properties->springConstant + powf(compression, power) * (forceMultiplier - 1) * properties->springConstant * powf(properties->radius, 1 - power);
	}

	void collideWithPlane(const PlaneDescriptor& plane);
	bool collideWithObstacle(GameObstacle& obstacle);
	// normal & separation are polar coordinates for the point on the obstacle (centred on the obstacle, in world space)
	void collideWithPointOnObstacle(const GameObstacle& obstacle, vec3 normal, float separation);

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

class EditorBall {
public:
	EditorBall() = default;

	explicit EditorBall(BallDescriptor* descriptor) :
		descriptor(descriptor),
		texture(getBallTexture(descriptor->getType())) {}

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }

private:
	BallDescriptor* descriptor{};

	Texture* texture{};

	bool selected = false;
};

#endif // BALL_H
