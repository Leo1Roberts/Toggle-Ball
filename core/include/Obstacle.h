#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "ButtonManager.h"
#include "Mesh.h"
#include "Obstacle.h"
#include "PhysicsConstants.h"
#include "Sizes.h"
#include "Smoother.h"

enum class SelectionType {
	Replace,
	Add,
	Subtract
};

struct SelectBox {
	float top, bottom, left, right;
	SelectionType selectionType;
};

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

constexpr float MINIMUM_ANGLE = -5 * PI; // -900°
constexpr float MAXIMUM_ANGLE = 5 * PI;  // 900°

constexpr float MINIMUM_RPM = -120;
constexpr float MAXIMUM_RPM = 120;

constexpr float MINIMUM_OPM = MINIMUM_RPM * 0.5f;
constexpr float MAXIMUM_OPM = MAXIMUM_RPM * 0.5f;

constexpr int SECTORS_PER_SEMICIRCLE = 64;
constexpr int SECTORS_PER_CIRCLE = SECTORS_PER_SEMICIRCLE * 2;
constexpr int SECTORS_PER_DOT = 8;
constexpr float BEVEL_AMOUNT = 0.1f;


class ObstacleKinematicState {
public:
	ObstacleKinematicState() = default;
	ObstacleKinematicState(vec3 position, float angle, vec3 velocity, float angularVelocity) :
	    position(position),
	    velocity(velocity),
	    angularVelocity(angularVelocity) {
		setAngle(angle);
	}

	void setPosition(vec3 pos) { position = pos; }
	void setAngle(float radians);
	void setVelocity(vec3 vel) { velocity = vel; }
	void setAngularVelocity(float angVel) { angularVelocity = angVel; }
	void setPhase(float radians) { phase = wrapAngle(radians); }

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] mat3 getRotation() const { return rotation; }
	[[nodiscard]] vec3 getVelocity() const { return velocity; }
	[[nodiscard]] float getAngularVelocity() const { return angularVelocity; }
	[[nodiscard]] float getPhase() const { return phase; }

private:
	vec3 position;
	float angle{};
	mat3 rotation = mat3::I;
	vec3 velocity;
	float angularVelocity{};
	float phase{};
};

class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

	[[nodiscard]] virtual std::unique_ptr<AbstractShapeSpec> clone() const = 0;

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<AbstractShapeSpec> deserialize(const std::string& data);

	virtual void scale(float factor) = 0;

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh) const;
	virtual void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

	[[nodiscard]] bool isInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const;

	[[nodiscard]] float getMinorRadius() const { return minorRadius; }
	[[nodiscard]] vec3 getLeftCap() const { return leftCap; }
	[[nodiscard]] vec3 getRightCap() const { return rightCap; }

protected:
	vec3 leftCap, rightCap;

	explicit AbstractShapeSpec(float minorRadius) :
	    minorRadius(minorRadius) {}

	AbstractShapeSpec(const AbstractShapeSpec&) = default;
	AbstractShapeSpec& operator=(const AbstractShapeSpec&) = default;
	AbstractShapeSpec(AbstractShapeSpec&&) = default;
	AbstractShapeSpec& operator=(AbstractShapeSpec&&) = default;

	[[nodiscard]] float getBevel() const { return BEVEL_AMOUNT * getMinorRadius(); }
	[[nodiscard]] float getHalfDepth() const { return getMinorRadius(); }
	[[nodiscard]] float getOutlineRadius() const { return getMinorRadius() + OUTLINE_WIDTH_WORLD; }

private:
	float minorRadius; // SSOT

	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	virtual void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const = 0;
	virtual void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

	[[nodiscard]] virtual bool midsectionIsInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const = 0;
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

	void setLeftLength(float l);
	void setRightLength(float l);

	[[nodiscard]] float getLeftLength() const { return leftLength; }
	[[nodiscard]] float getRightLength() const { return rightLength; }

private:
	float leftLength{};  // SSOT
	float rightLength{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "segment"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const override;
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

	void setArcAngle(float radians);
	void setArcRadius(float r);

	[[nodiscard]] float getArcAngle() const { return arcAngle; }
	[[nodiscard]] float getHalfArcAngle() const { return arcAngle / 2; }
	[[nodiscard]] float getArcRadius() const { return arcRadius; }

private:
	float arcAngle{};  // SSOT
	float arcRadius{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "arc"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const ObstacleKinematicState& s, const SelectBox& box) const override;

	void setCaps();
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

	virtual void scale(float factor) = 0;

	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec) const;

	// Initialises the kinematic state
	virtual void initKinematicState(ObstacleKinematicState& kinematicState) const = 0;
	// Updates the kinematic state by one physics frame. Purely incremental - kinematicState must be initialised separately.
	virtual void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const = 0;
	// Updates the (stationary) kinematic state for obstacles in the editor. Purely incremental - kinematicState must be initialised separately.
	virtual void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const = 0;

	[[nodiscard]] virtual const col& getColor() const = 0;

protected:
	IMotionSpec() = default;

private:
	[[nodiscard]] virtual std::string serializeData() const = 0;
	[[nodiscard]] virtual std::string getTypeString() const = 0;

	virtual void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const = 0;
};

class StaticSpec : public IMotionSpec {
public:
	StaticSpec(vec2 position, float angle) :
	    position(planarToWorld(position)),
	    angle(angle) {}

	StaticSpec(const std::string& data);

	~StaticSpec() override = default;

	[[nodiscard]] const col& getColor() const override { return WHITE; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<StaticSpec>(*this);
	}

	void scale(float factor) override {
		*this = StaticSpec(worldToPlanar(position) * factor, angle);
	}

	void setAngle(float radians);

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState&, const Smoother&) const override {}
	void updateEditorKinematicState(ObstacleKinematicState&, const Smoother&) const override {}

private:
	vec3 position; // SSOT
	float angle{}; // SSOT
	mat3 rotation;

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "static"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildDomainMesh(std::vector<ObjectVertex>&, std::vector<Index>&, const AbstractShapeSpec*) const override {}

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] mat3 getRotation() const { return rotation; }
};

class TogglingPositionSpec : public IMotionSpec {
public:
	TogglingPositionSpec(float angle, vec2 positionA, vec2 positionB) :
	    angle(angle),
	    positionA(planarToWorld(positionA)),
	    positionB(planarToWorld(positionB)) {
		setAngle(angle);
	}

	TogglingPositionSpec(const std::string& data);

	~TogglingPositionSpec() override = default;

	[[nodiscard]] const col& getColor() const override { return SOFT_BLUE; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingPositionSpec>(*this);
	}

	void scale(float factor) override {
		*this = TogglingPositionSpec(angle, worldToPlanar(positionA) * factor, worldToPlanar(positionB) * factor);
	}

	void setAngle(float radians);

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	float angle{}; // SSOT
	mat3 rotation;
	vec3 positionA; // SSOT
	vec3 positionB; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "t_position"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] mat3 getRotation() const { return rotation; }
	[[nodiscard]] vec3 getPositionA() const { return positionA; }
	[[nodiscard]] vec3 getPositionB() const { return positionB; }
};

class TogglingAngleSpec : public IMotionSpec {
public:
	TogglingAngleSpec(vec2 position, float angleA, float angleB) :
	    position(planarToWorld(position)),
	    angleA(angleA),
	    angleB(angleB) {}

	TogglingAngleSpec(const std::string& data);

	~TogglingAngleSpec() override = default;

	[[nodiscard]] const col& getColor() const override { return SOFT_RED; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingAngleSpec>(*this);
	}

	void scale(float factor) override {
		*this = TogglingAngleSpec(worldToPlanar(position) * factor, angleA, angleB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position;  // SSOT
	float angleA{}; // SSOT
	float angleB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "t_angle"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngleA() const { return angleA; }
	[[nodiscard]] float getAngleB() const { return angleB; }
};

class SpinningSpec : public IMotionSpec {
public:
	SpinningSpec(vec2 position, float initialAngle, float angularVelocityA, float angularVelocityB) :
	    position(planarToWorld(position)),
	    initialAngle(initialAngle),
	    angularVelocityA(angularVelocityA),
	    angularVelocityB(angularVelocityB) {}

	SpinningSpec(const std::string& data);

	~SpinningSpec() override = default;

	[[nodiscard]] const col& getColor() const override { return SOFT_MAGENTA; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<SpinningSpec>(*this);
	}

	void scale(float factor) override {
		*this = SpinningSpec(worldToPlanar(position) * factor, initialAngle, angularVelocityA, angularVelocityB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother&) const override;

private:
	vec3 position;            // SSOT
	float initialAngle{};     // SSOT
	float angularVelocityA{}; // SSOT
	float angularVelocityB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "spinning"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getInitialAngle() const { return initialAngle; }
	[[nodiscard]] float getAngularVelocityA() const { return angularVelocityA; }
	[[nodiscard]] float getAngularVelocityB() const { return angularVelocityB; }
};

class OscillatingPositionSpec : public IMotionSpec {
public:
	OscillatingPositionSpec(float angle, vec2 position1, vec2 position2, float angularFrequencyA, float angularFrequencyB) :
	    position1(planarToWorld(position1)),
	    position2(planarToWorld(position2)),
	    angle(angle),
	    angularFrequencyA(angularFrequencyA),
	    angularFrequencyB(angularFrequencyB) {}

	OscillatingPositionSpec(const std::string& data);

	~OscillatingPositionSpec() override = default;

	[[nodiscard]] const col& getColor() const override { return SOFT_CYAN; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingPositionSpec>(*this);
	}

	void scale(float factor) override {
		*this = OscillatingPositionSpec(angle, worldToPlanar(position1) * factor, worldToPlanar(position2) * factor, angularFrequencyA, angularFrequencyB);
	}

	void setAngle(float radians);

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position1; // SSOT
	vec3 position2; // SSOT
	float angle{};  // SSOT
	mat3 rotation;
	float angularFrequencyA{}; // SSOT
	float angularFrequencyB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "o_position"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition1() const { return position1; }
	[[nodiscard]] vec3 getPosition2() const { return position2; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] mat3 getRotation() const { return rotation; }
	[[nodiscard]] float getAngularFrequencyA() const { return angularFrequencyA; }
	[[nodiscard]] float getAngularFrequencyB() const { return angularFrequencyB; }
};

class OscillatingAngleSpec : public IMotionSpec {
public:
	OscillatingAngleSpec(vec2 position, float angle1, float angle2, float angularFrequencyA, float angularFrequencyB) :
	    position(planarToWorld(position)),
	    angle1(angle1),
	    angle2(angle2),
	    angularFrequencyA(angularFrequencyA),
	    angularFrequencyB(angularFrequencyB) {}

	OscillatingAngleSpec(const std::string& data);

	~OscillatingAngleSpec() override = default;

	[[nodiscard]] const col& getColor() const override { return SOFT_YELLOW; }

	[[nodiscard]] std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingAngleSpec>(*this);
	}

	void scale(float factor) override {
		*this = OscillatingAngleSpec(worldToPlanar(position) * factor, angle1, angle2, angularFrequencyA, angularFrequencyB);
	}

	void initKinematicState(ObstacleKinematicState& kinematicState) const override;
	void stepKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;
	void updateEditorKinematicState(ObstacleKinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position;             // SSOT
	float angle1{};            // SSOT
	float angle2{};            // SSOT
	float angularFrequencyA{}; // SSOT
	float angularFrequencyB{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class IMotionSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "o_angle"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle1() const { return angle1; }
	[[nodiscard]] float getAngle2() const { return angle2; }
	[[nodiscard]] float getAngularFrequencyA() const { return angularFrequencyA; }
	[[nodiscard]] float getAngularFrequencyB() const { return angularFrequencyB; }
};


class ObstacleDescriptor {
public:
	ObstacleDescriptor(std::unique_ptr<AbstractShapeSpec> shape, std::unique_ptr<IMotionSpec> motion, bool goal = false) :
	    shape(std::move(shape)),
	    motion(std::move(motion)),
	    goal(goal),
		material(MAT_CONCRETE) {}

	ObstacleDescriptor(const ObstacleDescriptor& other);
	ObstacleDescriptor& operator=(const ObstacleDescriptor& other);

	ObstacleDescriptor(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	void scale(float factor);

	[[nodiscard]] IMotionSpec* getMotion() const { return motion.get(); }
	[[nodiscard]] AbstractShapeSpec* getShape() const { return shape.get(); }
	[[nodiscard]] bool isGoal() const { return goal; }
	[[nodiscard]] byte getMaterial() const { return material; }

private:
	std::unique_ptr<AbstractShapeSpec> shape;
	std::unique_ptr<IMotionSpec> motion;
	bool goal{};
	byte material;
};


class GameObstacle {
public:
	explicit GameObstacle(const ObstacleDescriptor* descriptor) :
	    descriptor(descriptor) {
		reset();
		descriptor->getShape()->generateObstacleMesh(mesh, descriptor->getMotion()->getColor());
	}

	~GameObstacle() = default;

	GameObstacle(const GameObstacle& other) = delete;
	GameObstacle& operator=(const GameObstacle&) = delete;
	GameObstacle(GameObstacle&&) = default;
	GameObstacle& operator=(GameObstacle&&) = default;

	void reset() { descriptor->getMotion()->initKinematicState(kinematicState); }
	void updateKinematicState(const Smoother& smoother, int numSteps);

	[[nodiscard]] const ObstacleKinematicState* getKinematicState() const { return &kinematicState; }
	[[nodiscard]] const Mesh<ObjectVertex>* getMesh() const { return &mesh; }

private:
	const ObstacleDescriptor* descriptor;

	ObstacleKinematicState kinematicState;

	Mesh<ObjectVertex> mesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
};


class EditorObstacle {
public:
	explicit EditorObstacle(ObstacleDescriptor* descriptor) :
	    descriptor(descriptor) {
		descriptor->getMotion()->initKinematicState(kinematicState);
	}

	~EditorObstacle() = default;

	EditorObstacle(const EditorObstacle& other) = delete;
	EditorObstacle& operator=(const EditorObstacle&) = delete;
	EditorObstacle(EditorObstacle&&) = default;
	EditorObstacle& operator=(EditorObstacle&&) = default;

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }
	void setSelected(bool isSelected) { selected = isSelected; }

	// Only provide numSteps if demonstrating the continuous motion of an obstacle
	void updateKinematicState(const Smoother& smoother, int numSteps = -1);

private:
	ObstacleDescriptor* descriptor;

	ObstacleKinematicState kinematicState;

	bool selected = false;

	Mesh<ObjectVertex> obstacle = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> outline = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> domain = Mesh<ObjectVertex>(GL_STATIC_DRAW);
};

#endif // OBSTACLE_H