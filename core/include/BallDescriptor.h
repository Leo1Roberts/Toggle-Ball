#ifndef BALL_DESCRIPTOR_H
#define BALL_DESCRIPTOR_H

#include "PhysicsConstants.h"
#include "Texture.h"
#include "TypeAliases.h"
#include "Utilities.h"

struct BallKinematicState;


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

	bool operator==(const BallDescriptor&) const = default;

	void scale() {
		initialPosition *= getRadius();
	}

	void initKinematicState(BallKinematicState& kinematicState) const;

	[[nodiscard]] float getRadius() const { return ballProperties[type].radius; }

	byte type{};
	glm::vec2 initialPosition{0.f};
};


#endif // BALL_DESCRIPTOR_H
