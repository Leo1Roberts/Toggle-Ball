#ifndef BALL_DESCRIPTOR_H
#define BALL_DESCRIPTOR_H

#include "game/PhysicsConstants.h"
#include "opengl/Texture.h"
#include "TypeAlias.h"
#include "utilities/Utilities.h"

struct BallKinematicState;


enum class BallType : int {
	Basketball,
	Football,
	PingPong,
	Marble,
	COUNT
};

constexpr std::string getBallString(BallType type) {
	switch (type) {
	case BallType::Basketball: return "basketball";
	case BallType::Football:   return "football";
	case BallType::PingPong:   return "ping-pong";
	case BallType::Marble:     return "marble";
	default: return "";
	}
}

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

	byte material;               // SSOT

	float radius;                // SSOT
	float mass;                  // SSOT
	float momentOfInertia;       // SSOT
	float springConstant;        // SSOT
	float springConstantInv;
	float dampingCoefficient;    // SSOT
	float dampingCoefficientInv;
	float dragCoefficient;       // SSOT
};

constexpr BallProperties getBallProperties(BallType type) {
	switch (type) {
	default:
	case BallType::Basketball:
		return {
			MAT_BASKETBALL,
			0.12f,
			0.45f,
			0.65f * 0.45f * 0.12f * 0.12f,
			4000.f,
			4.f,
			0.5f
		};
	}
};

inline Texture* getBallTexture(BallType type) {
	switch (type) {
	case BallType::Basketball:
		return Textures::basketball.get();
	default:
		return Textures::white.get();
	}
}


struct BallDescriptor {
	constexpr BallDescriptor(BallType type, glm::vec2 initialPosition) :
		type(type),
		initialPosition(planarToWorld(initialPosition)) {}

	BallDescriptor(const std::string& data, float version);
	[[nodiscard]] std::string serialize() const;

	bool operator==(const BallDescriptor&) const = default;

	void scale() {
		initialPosition *= getRadius();
	}

	void initKinematicState(BallKinematicState& kinematicState) const;

	[[nodiscard]] constexpr float getRadius() const { return getBallProperties(type).radius; }

	BallType type = BallType::COUNT;
	glm::vec2 initialPosition{0.f};
};


#endif // BALL_DESCRIPTOR_H
