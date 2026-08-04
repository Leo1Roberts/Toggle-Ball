#ifndef OBSTACLE_MOTION_H
#define OBSTACLE_MOTION_H

#include "opengl/Mesh.h"

class AbstractShapeSpec;
class ObstacleKinematicState;
class Smoother;


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
	glm::vec2 position{0.f};   // SSOT
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


#endif // OBSTACLE_MOTION_H