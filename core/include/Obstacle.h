#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "Mesh.h"
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

enum {
	MAT_BASKETBALL,
	MAT_CONCRETE,
	MAT_NUM
};

constexpr float FRICTION_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
0.5f, 0.58f,
0.58f, 0.4f};

constexpr float ROLLING_RESISTANCE_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
0.02f, 0.015f,
0.015f, 0.01f};

constexpr float GRAVITY = -9.81f;
constexpr float AIR_DENSITY = 1.225f;
//const float DYNAMIC_VISCOSITY = 0.000018f;

constexpr vec3 OBSTACLE_ROTATION_AXIS = {1, 0, 0};

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
constexpr float MAXIMUM_ANGLE = 5 * PI; // 900°

constexpr float MINIMUM_RPM = -120;
constexpr float MAXIMUM_RPM = 120;

constexpr float MINIMUM_OPM = MINIMUM_RPM * 0.5f;
constexpr float MAXIMUM_OPM = MAXIMUM_RPM * 0.5f;

constexpr int SECTORS_PER_SEMICIRCLE = 64;
constexpr int SECTORS_PER_CIRCLE = SECTORS_PER_SEMICIRCLE * 2;
constexpr int SECTORS_PER_DOT = 8;
constexpr float BEVEL_AMOUNT = 0.1f;

inline vec3 planarToWorld(vec2 planarVec) {
	return {0, planarVec.x, planarVec.y};
}


class KinematicState {
public:
	KinematicState() = default;
	KinematicState(vec3 position, float angle, vec3 velocity, float angularVelocity) :
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
	vec3 position = vec3();
	float angle = 0;
	mat3 rotation = mat3::I;
	vec3 velocity = vec3();
	float angularVelocity = 0;
	float phase = 0;
};

class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

	virtual std::unique_ptr<AbstractShapeSpec> clone() const = 0;

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<AbstractShapeSpec> deserialize(const std::string& data);

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh) const;
	virtual void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

	[[nodiscard]] bool isInSelectBox(const KinematicState& s, const SelectBox& box) const;

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

	[[nodiscard]] virtual bool midsectionIsInSelectBox(const KinematicState& s, const SelectBox& box) const = 0;
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

	std::unique_ptr<AbstractShapeSpec> clone() const override {
		return std::make_unique<SegmentSpec>(*this);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void setLeftLength(float l);
	void setRightLength(float l);

	[[nodiscard]] float getLeftLength() const { return leftLength; }
	[[nodiscard]] float getRightLength() const { return rightLength; }

private:
	float leftLength{}; // SSOT
	float rightLength{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "segment"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const KinematicState& s, const SelectBox& box) const override;
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

	std::unique_ptr<AbstractShapeSpec> clone() const override {
		return std::make_unique<ArcSpec>(*this);
	}

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void setArcAngle(float radians);
	void setArcRadius(float r);

	[[nodiscard]] float getArcAngle() const { return arcAngle; }
	[[nodiscard]] float getHalfArcAngle() const { return arcAngle / 2; }
	[[nodiscard]] float getArcRadius() const { return arcRadius; }

private:
	float arcAngle{}; // SSOT
	float arcRadius{}; // SSOT

	[[nodiscard]] std::string serializeData() const override;
	friend class AbstractShapeSpec;
	[[nodiscard]] static std::string getTypeStringStatic() { return "arc"; }
	[[nodiscard]] std::string getTypeString() const override { return getTypeStringStatic(); }

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	[[nodiscard]] bool midsectionIsInSelectBox(const KinematicState& s, const SelectBox& box) const override;

	void setCaps();
};


class IMotionSpec {
public:
	virtual ~IMotionSpec() = default;

	IMotionSpec(const IMotionSpec&) = default;
	IMotionSpec& operator=(const IMotionSpec&) = default;
	IMotionSpec(IMotionSpec&&) = default;
	IMotionSpec& operator=(IMotionSpec&&) = default;

	virtual std::unique_ptr<IMotionSpec> clone() const = 0;

	[[nodiscard]] std::string serialize() const;
	static std::unique_ptr<IMotionSpec> deserialize(const std::string& data);

	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec) const;

	// Updates KinematicState by one physics frame. Purely incremental - KinematicState must be initialised separately.
	virtual void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const = 0;
	// Updates the (stationary) KinematicState for obstacles in the editor. Purely incremental - KinematicState must be initialised separately.
	virtual void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const = 0;

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

	std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<StaticSpec>(*this);
	}

	void setAngle(float radians);

	void stepKinematicState(KinematicState&, const Smoother&) const override {}

	void updateEditorKinematicState(KinematicState&, const Smoother&) const override {}

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

	std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingPositionSpec>(*this);
	}

	void setAngle(float radians);

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

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

	std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<TogglingAngleSpec>(*this);
	}

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position; // SSOT
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

	std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<SpinningSpec>(*this);
	}

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother&) const override;

private:
	vec3 position; // SSOT
	float initialAngle{}; // SSOT
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

	std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingPositionSpec>(*this);
	}

	void setAngle(float radians);

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position1; // SSOT
	vec3 position2; // SSOT
	float angle{}; // SSOT
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

	std::unique_ptr<IMotionSpec> clone() const override {
		return std::make_unique<OscillatingAngleSpec>(*this);
	}

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position; // SSOT
	float angle1{}; // SSOT
	float angle2{}; // SSOT
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
	    goal(goal) {}

	ObstacleDescriptor(const ObstacleDescriptor& other);
	ObstacleDescriptor& operator=(const ObstacleDescriptor& other);

	ObstacleDescriptor(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	[[nodiscard]] IMotionSpec* getMotion() const { return motion.get(); }
	[[nodiscard]] AbstractShapeSpec* getShape() const { return shape.get(); }
	[[nodiscard]] bool isGoal() const { return goal; }

private:
	std::unique_ptr<AbstractShapeSpec> shape;
	std::unique_ptr<IMotionSpec> motion;
	bool goal{};
};


class EditorObstacle {
public:
	explicit EditorObstacle(ObstacleDescriptor* descriptor) :
	    descriptor(descriptor) {}

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

	KinematicState kinematicState;

	bool selected = false;

	Mesh<ObjectVertex> obstacle = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> outline = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> domain = Mesh<ObjectVertex>(GL_STATIC_DRAW);
};

#endif // OBSTACLE_H