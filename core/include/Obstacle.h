#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "Ball.h"
#include "SelectBox.h"
#include "Mesh.h"
#include "PhysicsConstants.h"
#include "Plane.h"
#include "Smoother.h"
#include "Utilities.h"
#include "glm/ext/scalar_constants.hpp"

struct BallCollisionInfo;
class GameBall;
struct SelectBox;

// TODO: move the following constants to somewhere better

constexpr float MINIMUM_ARENA_SIZE = 5;
constexpr float MAXIMUM_ARENA_SIZE = 200;

constexpr float MINIMUM_POS_X = -MAXIMUM_ARENA_SIZE * 0.7f;
constexpr float MAXIMUM_POS_X = MAXIMUM_ARENA_SIZE * 0.7f;
constexpr float MINIMUM_POS_Y = -MAXIMUM_ARENA_SIZE * 0.2f;
constexpr float MAXIMUM_POS_Y = MAXIMUM_ARENA_SIZE * 1.2f;

constexpr float MINIMUM_TRANSITION_TIME = 0.1f;
constexpr float MAXIMUM_TRANSITION_TIME = 20;

constexpr float MINIMUM_MINOR_RADIUS = 0.25f;
constexpr float MAXIMUM_MINOR_RADIUS = 50;

constexpr float MAXIMUM_MAJOR_RADIUS = 200;

constexpr float MINIMUM_ANGLE = -5 * glm::pi<float>(); // -900°
constexpr float MAXIMUM_ANGLE = 5 * glm::pi<float>();  // 900°

constexpr float MINIMUM_RPM = -120;
constexpr float MAXIMUM_RPM = 120;

constexpr float MINIMUM_OPM = MINIMUM_RPM * 0.5f;
constexpr float MAXIMUM_OPM = MAXIMUM_RPM * 0.5f;

constexpr int SECTORS_PER_SEMICIRCLE = 64;
constexpr int SECTORS_PER_CIRCLE = SECTORS_PER_SEMICIRCLE * 2;
constexpr int SECTORS_PER_DOT = 8;
constexpr float BEVEL_AMOUNT = 0.1f;


inline glm::mat2 angleToRotation2D(float radians) {
	float c = std::cos(radians);
	float s = std::sin(radians);
	return { c,  s,
			-s,  c };
}


class ObstacleKinematicState {
public:
	ObstacleKinematicState() = default;
	ObstacleKinematicState(glm::vec3 position, float angle, glm::vec3 velocity, float angularVelocity) :
	    position(position),
	    velocity(velocity),
	    angularVelocity(angularVelocity) {
		setAngle(angle);
	}

	void setPosition(glm::vec3 pos) { position = pos; }
	void setAngle(float radians);
	void setVelocity(glm::vec3 vel) { velocity = vel; }
	void setAngularVelocity(float angVel) { angularVelocity = angVel; }
	void setPhase(float radians) { phase = wrapAngle(radians); }

	[[nodiscard]] glm::vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] glm::mat3 getRotation() const { return rotation; }
	[[nodiscard]] glm::vec3 getVelocity() const { return velocity; }
	[[nodiscard]] float getAngularVelocity() const { return angularVelocity; }
	[[nodiscard]] float getPhase() const { return phase; }

private:
	glm::vec3 position{0.f};
	float angle = 0.f;
	glm::mat3 rotation = glm::mat3(1.f);
	glm::vec3 velocity{0.f};
	float angularVelocity = 0.f;
	float phase = 0.f;
};

class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

	[[nodiscard]] virtual std::unique_ptr<AbstractShapeSpec> clone() const = 0;

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<AbstractShapeSpec> deserialize(const std::string& data);

	bool operator==(const AbstractShapeSpec& other) const;

	virtual void scale(float factor) = 0;

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh, float uiToWorldScale) const;
	virtual void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

	[[nodiscard]] bool isInSelectBox(const ObstacleKinematicState& s, SelectBox box) const;

	[[nodiscard]] virtual bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const = 0;
	// Only sets all members if .collision == true
	[[nodiscard]] virtual BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) = 0;

	[[nodiscard]] float getMinorRadius() const { return minorRadius; }
	[[nodiscard]] float getBoundingRadius() const { return getMajorRadius() + getMinorRadius(); }
	[[nodiscard]] glm::vec2 getLeftCap() const { return leftCap; }
	[[nodiscard]] glm::vec2 getRightCap() const { return rightCap; }
	[[nodiscard]] virtual float getLeftCapAngle() const = 0;
	[[nodiscard]] virtual float getRightCapAngle() const = 0;
	[[nodiscard]] float getHalfDepth() const { return getMinorRadius(); }

protected:
	glm::vec2 leftCap{}, rightCap{};

	explicit AbstractShapeSpec(float minorRadius) :
	    minorRadius(minorRadius) {}

	AbstractShapeSpec(const AbstractShapeSpec&) = default;
	AbstractShapeSpec& operator=(const AbstractShapeSpec&) = default;
	AbstractShapeSpec(AbstractShapeSpec&&) = default;
	AbstractShapeSpec& operator=(AbstractShapeSpec&&) = default;

	[[nodiscard]] float getBevel() const { return BEVEL_AMOUNT * getMinorRadius(); }

private:
	float minorRadius; // SSOT

	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	[[nodiscard]] virtual bool equals(const AbstractShapeSpec& other) const = 0;

	virtual void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const = 0;
	virtual void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const = 0;

	[[nodiscard]] virtual bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const = 0;

	[[nodiscard]] virtual float getMajorRadius() const = 0;
};

class SegmentSpec : public AbstractShapeSpec {
public:
	SegmentSpec(float minorRadius, float leftLength, float rightLength) :
	    AbstractShapeSpec(minorRadius) {
		setLeftLength(leftLength);
		setRightLength(rightLength);
	}
	SegmentSpec(float minorRadius, const std::string& data);

	~SegmentSpec() override = default;

	[[nodiscard]] std::unique_ptr<AbstractShapeSpec> clone() const override {
		return std::make_unique<SegmentSpec>(*this);
	}

	void scale(float factor) override {
		*this = SegmentSpec(getMinorRadius() * factor, leftLength * factor, rightLength * factor);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	[[nodiscard]] bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const override {
		return leftPlaneDistance <= 0 && rightPlaneDistance <= 0;
	}
	[[nodiscard]] BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) override;

	void setLeftLength(float len);
	void setRightLength(float len);

	[[nodiscard]] float getLeftCapAngle() const override { return glm::pi<float>(); }
	[[nodiscard]] float getRightCapAngle() const override { return 0; }
	[[nodiscard]] float getLeftLength() const { return leftLength; }
	[[nodiscard]] float getRightLength() const { return rightLength; }

private:
	float leftLength{};  // SSOT
	float rightLength{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "segment"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const AbstractShapeSpec& other) const override {
		auto otherSegment = (const SegmentSpec&)other;
		return leftLength == otherSegment.leftLength && rightLength == otherSegment.rightLength;
	}

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const override;
	[[nodiscard]] float getMajorRadius() const override { return std::max(leftLength, rightLength); }
	[[nodiscard]] PlaneDescriptor getTopPlane(const ObstacleKinematicState& kinematicState) const;
};

class ArcSpec : public AbstractShapeSpec {
public:
	ArcSpec(float minorRadius, float arcAngle, float arcRadius) :
	    AbstractShapeSpec(minorRadius),
	    arcAngle(arcAngle),
	    arcRadius(arcRadius) {
		setCaps();
	}
	ArcSpec(float minorRadius, const std::string& data);

	~ArcSpec() override = default;

	[[nodiscard]] std::unique_ptr<AbstractShapeSpec> clone() const override {
		return std::make_unique<ArcSpec>(*this);
	}

	void scale(float factor) override {
		*this = ArcSpec(getMinorRadius() * factor, arcAngle, arcRadius * factor);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	[[nodiscard]] bool pointIsBetweenCaps(float leftPlaneDistance, float rightPlaneDistance) const override {
		return getArcAngle() <= glm::pi<float>() && (leftPlaneDistance <= 0 && rightPlaneDistance <= 0) ||
			   getArcAngle() >  glm::pi<float>() && (leftPlaneDistance <= 0 || rightPlaneDistance <= 0);
	}
	[[nodiscard]] BallCollisionInfo getMidsectionCollision(const ObstacleKinematicState& kinematicState, const GameBall& ball) override;

	void setArcAngle(float radians);
	void setArcRadius(float r);

	[[nodiscard]] float getLeftCapAngle() const override { return getHalfArcAngle() + glm::pi<float>(); }
	[[nodiscard]] float getRightCapAngle() const override { return -getHalfArcAngle(); }
	[[nodiscard]] float getArcAngle() const { return arcAngle; }
	[[nodiscard]] float getHalfArcAngle() const { return arcAngle / 2.f; }
	[[nodiscard]] float getArcRadius() const { return arcRadius; }

private:
	float arcAngle{};  // SSOT
	float arcRadius{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "arc"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const AbstractShapeSpec& other) const override {
		auto otherArc = (const ArcSpec&)other;
		return arcAngle == otherArc.arcAngle && arcRadius == otherArc.arcRadius;
	}

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, float uiToWorldScale) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, SelectBox box) const override;

	void setCaps();

	[[nodiscard]] float getMajorRadius() const override { return arcRadius; }
};


class IMotionSpec {
public:
	virtual ~IMotionSpec() = default;

	IMotionSpec(const IMotionSpec&) = default;
	IMotionSpec& operator=(const IMotionSpec&) = default;
	IMotionSpec(IMotionSpec&&) = default;
	IMotionSpec& operator=(IMotionSpec&&) = default;

	[[nodiscard]] virtual std::unique_ptr<IMotionSpec> clone() const = 0;

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<IMotionSpec> deserialize(const std::string& data);

	bool operator==(const IMotionSpec& other) const;

	virtual void scale(float factor) = 0;

	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const;

	// Initialises the kinematic state
	virtual void initKinematicState(ObstacleKinematicState& kinematicState) const = 0;
	// Updates the kinematic state by one physics frame. Purely incremental - kinematicState must be initialised separately.
	virtual void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const = 0;
	// Updates the (stationary) kinematic state for obstacles in the editor. Purely incremental - kinematicState must be initialised separately.
	virtual void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const = 0;


	virtual void translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) = 0;

	[[nodiscard]] virtual col getColor() const = 0;
	[[nodiscard]] virtual glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const = 0;

protected:
	IMotionSpec() = default;

private:
	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	[[nodiscard]] virtual bool equals(const IMotionSpec& other) const = 0;

	virtual void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const = 0;
};

class StaticSpec : public IMotionSpec {
public:
	StaticSpec(glm::vec2 position, float angle) :
	    position(position) {
		setAngle(angle);
	}

	explicit StaticSpec(const std::string& data);

	~StaticSpec() override = default;

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const StaticSpec*)base)->position + vector;
	}

	[[nodiscard]] col getColor() const override { return Color::White; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<StaticSpec>(*this);
	}

	void scale(float factor) override {
		*this = StaticSpec(position * factor, angle);
	}

	void setAngle(float radians);

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState&, const Smoother&) const override {}
	void updateEditorKinematicState(ObstacleKinematicState&, const Smoother&) const override {}

private:
	glm::vec2 position{0.f}; // SSOT
	float angle{}; // SSOT
	glm::mat2 rotation{1.f};

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "static"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherStaticSpec = (const StaticSpec&)other;
		return position == otherStaticSpec.position && angle == otherStaticSpec.angle;
	}

	void buildDomainMesh(std::vector<ObjectVertex>&, std::vector<Index>&, const AbstractShapeSpec*, float) const override {}

	[[nodiscard]] glm::vec2 getPosition() const { return position; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] glm::mat2 getRotation() const { return rotation; }
};

class TogglingPositionSpec : public IMotionSpec {
public:
	TogglingPositionSpec(float angle, glm::vec2 positionA, glm::vec2 positionB) :
	    positionA(positionA),
	    positionB(positionB) {
		setAngle(angle);
	}

	explicit TogglingPositionSpec(const std::string& data);

	~TogglingPositionSpec() override = default;

	void translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) override;

	[[nodiscard]] col getColor() const override { return Color::SoftBlue; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2) const override { return glm::vec2(0.f); }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingPositionSpec>(*this);
	}

	void scale(float factor) override {
		*this = TogglingPositionSpec(angle, positionA * factor, positionB * factor);
	}

	void setAngle(float radians);

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	float angle{}; // SSOT
	glm::mat2 rotation{1.f};
	glm::vec2 positionA{0.f}; // SSOT
	glm::vec2 positionB{0.f}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "t_position"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherTogglingPositionSpec = (const TogglingPositionSpec&)other;
		return angle == otherTogglingPositionSpec.angle && positionA == otherTogglingPositionSpec.positionA && positionB == otherTogglingPositionSpec.positionB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const override;

	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] glm::mat2 getRotation() const { return rotation; }
	[[nodiscard]] glm::vec2 getPositionA() const { return positionA; }
	[[nodiscard]] glm::vec2 getPositionB() const { return positionB; }
};

class TogglingAngleSpec : public IMotionSpec {
public:
	TogglingAngleSpec(glm::vec2 position, float angleA, float angleB) :
	    position(position),
	    angleA(angleA),
	    angleB(angleB) {}

	explicit TogglingAngleSpec(const std::string& data);

	~TogglingAngleSpec() override = default;

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const TogglingAngleSpec*)base)->position + vector;
	}

	[[nodiscard]] col getColor() const override { return Color::SoftRed; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingAngleSpec>(*this);
	}

	void scale(float factor) override {
		*this = TogglingAngleSpec(position * factor, angleA, angleB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	glm::vec2 position{0.f};  // SSOT
	float angleA{}; // SSOT
	float angleB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "t_angle"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherTogglingAngleSpec = (const TogglingAngleSpec&)other;
		return position == otherTogglingAngleSpec.position && angleA == otherTogglingAngleSpec.angleA && angleB == otherTogglingAngleSpec.angleB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float uiToWorldScale) const override;

	[[nodiscard]] glm::vec2 getPosition() const { return position; }
	[[nodiscard]] float getAngleA() const { return angleA; }
	[[nodiscard]] float getAngleB() const { return angleB; }
};

class SpinningSpec : public IMotionSpec {
public:
	SpinningSpec(glm::vec2 position, float initialAngle, float angularVelocityA, float angularVelocityB) :
	    position(position),
	    initialAngle(initialAngle),
	    angularVelocityA(angularVelocityA),
	    angularVelocityB(angularVelocityB) {}

	explicit SpinningSpec(const std::string& data);

	~SpinningSpec() override = default;

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const SpinningSpec*)base)->position + vector;
	}

	[[nodiscard]] col getColor() const override { return Color::SoftMagenta; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<SpinningSpec>(*this);
	}

	void scale(float factor) override {
		*this = SpinningSpec(position * factor, initialAngle, angularVelocityA, angularVelocityB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother&) const override;

private:
	glm::vec2 position{0.f};  // SSOT
	float initialAngle{};     // SSOT
	float angularVelocityA{}; // SSOT
	float angularVelocityB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "spinning"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherSpinningSpec = (const SpinningSpec&)other;
		return
			position == otherSpinningSpec.position && initialAngle == otherSpinningSpec.initialAngle &&
			angularVelocityA == otherSpinningSpec.angularVelocityA && angularVelocityB == otherSpinningSpec.angularVelocityB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const override;

	[[nodiscard]] glm::vec2 getPosition() const { return position; }
	[[nodiscard]] float getInitialAngle() const { return initialAngle; }
	[[nodiscard]] float getAngularVelocityA() const { return angularVelocityA; }
	[[nodiscard]] float getAngularVelocityB() const { return angularVelocityB; }
};

class OscillatingPositionSpec : public IMotionSpec {
public:
	OscillatingPositionSpec(float angle, glm::vec2 position1, glm::vec2 position2, float angularFrequencyA, float angularFrequencyB) :
	    position1(position1),
	    position2(position2),
	    angularFrequencyA(angularFrequencyA),
	    angularFrequencyB(angularFrequencyB) {
		setAngle(angle);
	}

	explicit OscillatingPositionSpec(const std::string& data);

	~OscillatingPositionSpec() override = default;

	void translateBy(glm::vec2 vector, bool stateless, bool toggled, const IMotionSpec* base) override;

	[[nodiscard]] col getColor() const override { return Color::SoftCyan; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2) const override { return glm::vec2(0.f); }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingPositionSpec>(*this);
	}

	void scale(float factor) override {
		*this = OscillatingPositionSpec(angle, position1 * factor, position2 * factor, angularFrequencyA, angularFrequencyB);
	}

	void setAngle(float radians);

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	glm::vec2 position1{0.f};  // SSOT
	glm::vec2 position2{0.f};  // SSOT
	float angle{};             // SSOT
	glm::mat2 rotation{1.f};
	float angularFrequencyA{}; // SSOT
	float angularFrequencyB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "o_position"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherOscillatingPositionSpec = (const OscillatingPositionSpec&)other;
		return
			position1 == otherOscillatingPositionSpec.position1 && position2 == otherOscillatingPositionSpec.position2 &&
			angle == otherOscillatingPositionSpec.angle &&
			angularFrequencyA == otherOscillatingPositionSpec.angularFrequencyA && angularFrequencyB == otherOscillatingPositionSpec.angularFrequencyB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const override;

	[[nodiscard]] glm::vec2 getPosition1() const { return position1; }
	[[nodiscard]] glm::vec2 getPosition2() const { return position2; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] glm::mat2 getRotation() const { return rotation; }
	[[nodiscard]] float getAngularFrequencyA() const { return angularFrequencyA; }
	[[nodiscard]] float getAngularFrequencyB() const { return angularFrequencyB; }
};

class OscillatingAngleSpec : public IMotionSpec {
public:
	OscillatingAngleSpec(glm::vec2 position, float angle1, float angle2, float angularFrequencyA, float angularFrequencyB) :
	    position(position),
	    angle1(angle1),
	    angle2(angle2),
	    angularFrequencyA(angularFrequencyA),
	    angularFrequencyB(angularFrequencyB) {}

	OscillatingAngleSpec(const std::string& data);

	~OscillatingAngleSpec() override = default;

	void translateBy(glm::vec2 vector, bool, bool, const IMotionSpec* base) override {
		position = ((const OscillatingAngleSpec*)base)->position + vector;
	}

	[[nodiscard]] col getColor() const override { return Color::SoftYellow; }
	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const override { return obstaclePosition; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingAngleSpec>(*this);
	}

	void scale(float factor) override {
		*this = OscillatingAngleSpec(position * factor, angle1, angle2, angularFrequencyA, angularFrequencyB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	glm::vec2 position{0.f};             // SSOT
	float angle1{};            // SSOT
	float angle2{};            // SSOT
	float angularFrequencyA{}; // SSOT
	float angularFrequencyB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "o_angle"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	[[nodiscard]] bool equals(const IMotionSpec& other) const override {
		auto otherOscillatingAngleSpec = (const OscillatingAngleSpec&)other;
		return
			position == otherOscillatingAngleSpec.position &&
			angle1 == otherOscillatingAngleSpec.angle1 && angle2 == otherOscillatingAngleSpec.angle2 &&
			angularFrequencyA == otherOscillatingAngleSpec.angularFrequencyA && angularFrequencyB == otherOscillatingAngleSpec.angularFrequencyB;
	}

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec, float) const override;

	[[nodiscard]] glm::vec2 getPosition() const { return position; }
	[[nodiscard]] float getAngle1() const { return angle1; }
	[[nodiscard]] float getAngle2() const { return angle2; }
	[[nodiscard]] float getAngularFrequencyA() const { return angularFrequencyA; }
	[[nodiscard]] float getAngularFrequencyB() const { return angularFrequencyB; }
};


struct ObstacleDescriptor {
	ObstacleDescriptor(std::unique_ptr<AbstractShapeSpec> shape, std::unique_ptr<IMotionSpec> motion, bool goal = false) :
	    shape(std::move(shape)),
	    motion(std::move(motion)),
	    goal(goal),
		color(goal ? Color::SoftGreen : this->motion->getColor()),
		material(MAT_CONCRETE) {}

	ObstacleDescriptor(const ObstacleDescriptor& other);
	ObstacleDescriptor& operator=(const ObstacleDescriptor& other);

	ObstacleDescriptor(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	bool operator==(const ObstacleDescriptor& other) const;

	void scale(float factor);

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh) const {
		shape->generateObstacleMesh(obstacleMesh, color);
	}
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh, float uiToWorldScale) const {
		shape->generateOutlineMesh(outlineMesh, uiToWorldScale);
	}
	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, float uiToWorldScale) const {
		motion->generateDomainMesh(domainMesh, shape.get(), uiToWorldScale);
	}

	[[nodiscard]] glm::vec2 getDomainPosition(glm::vec2 obstaclePosition) const {
		return motion->getDomainPosition(obstaclePosition);
	}

	std::unique_ptr<AbstractShapeSpec> shape;
	std::unique_ptr<IMotionSpec> motion;
	bool goal = false;
	col color;
	byte material;
};


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


class EditorObstacle {
public:
	explicit EditorObstacle(ObstacleDescriptor* descriptor) :
	    descriptor(descriptor) {
		initKinematicState();
		descriptor->generateObstacleMesh(obstacleMesh);
	}

	~EditorObstacle() = default;

	EditorObstacle(const EditorObstacle& other) = delete;
	EditorObstacle& operator=(const EditorObstacle&) = delete;
	EditorObstacle(EditorObstacle&&) = default;
	EditorObstacle& operator=(EditorObstacle&&) = default;

	void generateMeshes(float uiToWorldScale) {
		descriptor->generateObstacleMesh(obstacleMesh);
		generateEphemeralMeshes(uiToWorldScale);
	}
	void generateEphemeralMeshes(float uiToWorldScale) {
		descriptor->generateOutlineMesh(outlineMesh, uiToWorldScale);
		generateDomainMesh(uiToWorldScale);
	}
	void generateDomainMesh(float uiToWorldScale) {
		descriptor->generateDomainMesh(domainMesh, uiToWorldScale);
	}

	void initKinematicState() { descriptor->motion->initKinematicState(kinematicState); }
	// Only provide numSteps if demonstrating the continuous motion of an obstacle
	void updateKinematicState(const Smoother& smoother, int numSteps = -1);

	void translateBy(glm::vec2 vector, bool stateless, bool toggled, const ObstacleDescriptor* base) {
		descriptor->motion->translateBy(vector, stateless, toggled, base->motion.get());
	}

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }
	void setSelected(bool select) { selected = select; }
	[[nodiscard]] bool isInSelectBox(SelectBox box) const { return descriptor->shape->isInSelectBox(kinematicState, box); }

	[[nodiscard]] const ObstacleDescriptor* getDescriptor() const { return descriptor; }
	[[nodiscard]] const ObstacleKinematicState* getKinematicState() const { return &kinematicState; }
	[[nodiscard]] const Mesh<ObjectVertex>* getObstacleMesh() const { return &obstacleMesh; }
	[[nodiscard]] const Mesh<ObjectVertex>* getOutlineMesh() const { return &outlineMesh; }
	[[nodiscard]] const Mesh<ObjectVertex>* getDomainMesh() const { return &domainMesh; }

	[[nodiscard]] glm::vec2 getDomainPosition() const { return descriptor->getDomainPosition(worldToPlanar(kinematicState.getPosition())); }

private:
	ObstacleDescriptor* descriptor;

	ObstacleKinematicState kinematicState;

	bool selected = false;

	Mesh<ObjectVertex> obstacleMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> outlineMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> domainMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
};

#endif // OBSTACLE_H