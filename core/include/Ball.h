#ifndef BALL_H
#define BALL_H

#include "SelectBox.h"
#include "Mesh.h"
#include "Obstacle.h"
#include "PhysicsConstants.h"
#include "Plane.h"
#include "Settings.h"
#include "Texture.h"
#include "Utilities.h"

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
	glm::vec3 position{0.f};
	glm::mat3 rotation{1.f};
	glm::vec3 velocity{0.f};
	glm::vec3 angularVelocity{0.f};
};

struct BallForces {
	glm::vec3 force{0.f};
	glm::vec3 torque{0.f};

	void reset() { force = torque = glm::vec3(0.f); }
};

struct BallCollisionInfo {
	bool colliding{};
	glm::vec3 normal{};
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



struct BallDescriptor {
	constexpr BallDescriptor(byte type, glm::vec2 initialPosition) :
		type(type),
		initialPosition(planarToWorld(initialPosition)) {}

	BallDescriptor(const std::string& data, float version);
	[[nodiscard]] std::string serialize() const;

	void scale() {
		initialPosition *= getRadius();
	}

	void initKinematicState(BallKinematicState& kinematicState) const;

	[[nodiscard]] float getRadius() const { return ballProperties[type].radius; }

	byte type{};
	glm::vec2 initialPosition{0.f};
};

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

class EditorBall {
public:
	EditorBall() = default;

	explicit EditorBall(BallDescriptor* descriptor) :
		descriptor(descriptor),
		texture(getBallTexture(descriptor->type)) {}

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }
	void setSelected(bool select) { selected = select; }
	[[nodiscard]] bool isInSelectBox(SelectBox selectBox) const {
		return selectBox.touchesCircle(descriptor->initialPosition, 1.f);
	}

	void updateOutlineRadius(float uiToWorldScale) {
		outlineRadius = 1.f + Settings::Sizes.outlineWidth * uiToWorldScale;
	}
	[[nodiscard]] float getOutlineRadius() const { return outlineRadius; }

	void translateBy(glm::vec2 vector, const BallDescriptor* base) {
		descriptor->initialPosition = base->initialPosition + vector;
	}

	[[nodiscard]] const BallDescriptor* getDescriptor() const { return descriptor; }
	[[nodiscard]] const Texture* getTexture() const { return texture; }

private:
	BallDescriptor* descriptor{};

	Texture* texture{};

	bool selected = false;

	float outlineRadius = 1.f;
};

#endif // BALL_H
