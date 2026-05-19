#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "Mesh.h"
#include "Sizes.h"
#include "Smoother.h"

enum {
	MAT_BASKETBALL,
	MAT_CONCRETE,
	MAT_NUM
};

const float FRICTION_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
0.5f, 0.58f,
0.58f, 0.4f};

const float ROLLING_RESISTANCE_COEFFICIENTS[MAT_NUM][MAT_NUM] = {
0.02f, 0.015f,
0.015f, 0.01f};

const float GRAVITY = -9.81f;
const float AIR_DENSITY = 1.225f;
//const float DYNAMIC_VISCOSITY = 0.000018f;

const vec3 OBSTACLE_ROTATION_AXIS = {1, 0, 0};

const float MINIMUM_ARENA_SIZE = 5;
const float MAXIMUM_ARENA_SIZE = 200;

const float MINIMUM_POS_X = -MAXIMUM_ARENA_SIZE * 0.7f;
const float MAXIMUM_POS_X = MAXIMUM_ARENA_SIZE * 0.7f;
const float MINIMUM_POS_Y = -MAXIMUM_ARENA_SIZE * 0.2f;
const float MAXIMUM_POS_Y = MAXIMUM_ARENA_SIZE * 1.2f;

const float MINIMUM_TRANSITION_TIME = 0.1f;
const float MAXIMUM_TRANSITION_TIME = 20;

const float MINIMUM_MINOR_RADIUS = 0.25f;
const float MAXIMUM_MINOR_RADIUS = 50;

const float MAXIMUM_MAJOR_RADIUS = 200;

const float MINIMUM_ANGLE = -5 * PI; // -900°
const float MAXIMUM_ANGLE = 5 * PI; // 900°

const float MINIMUM_RPM = -120;
const float MAXIMUM_RPM = 120;

const float MINIMUM_OPM = MINIMUM_RPM * 0.5f;
const float MAXIMUM_OPM = MAXIMUM_RPM * 0.5f;

const int SECTORS_PER_SEMICIRCLE = 64;
const int SECTORS_PER_CIRCLE = SECTORS_PER_SEMICIRCLE * 2;
const int SECTORS_PER_DOT = 8;
const float BEVEL_AMOUNT = 0.1f;

inline vec3 planarToWorld(vec2 planarVec) {
	return vec3(0, planarVec.x, planarVec.y);
}

class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh, col color) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh) const;
	virtual void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;

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
	float minorRadius;

	virtual void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const = 0;
	virtual void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;
};

class SegmentSpec : public AbstractShapeSpec {
public:
	SegmentSpec(float minorRadius, float leftLength, float rightLength) :
	    AbstractShapeSpec(minorRadius) {
		setLeftLength(leftLength);
		setRightLength(rightLength);
	}

	~SegmentSpec() override = default;

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void setLeftLength(float l);
	void setRightLength(float l);

	[[nodiscard]] float getLeftLength() const { return leftLength; }
	[[nodiscard]] float getRightLength() const { return rightLength; }

private:
	float leftLength;
	float rightLength;

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;
};

class ArcSpec : public AbstractShapeSpec {
public:
	ArcSpec(float minorRadius, float arcAngle, float arcRadius) :
	    AbstractShapeSpec(minorRadius),
	    arcAngle(arcAngle),
	    arcRadius(arcRadius) {
		setCaps();
	}

	~ArcSpec() override = default;

	void buildShadowMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void setArcAngle(float radians);
	void setArcRadius(float r);

	[[nodiscard]] float getArcAngle() const { return arcAngle; }
	[[nodiscard]] float getHalfArcAngle() const { return arcAngle / 2; }
	[[nodiscard]] float getArcRadius() const { return arcRadius; }

private:
	float arcAngle;
	float arcRadius;

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, col color) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;

	void setCaps();
};


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
	void setAngle(float radians) { angle = wrapAngle(radians); }
	void setVelocity(vec3 vel) { velocity = vel; }
	void setAngularVelocity(float angVel) { angularVelocity = angVel; }
	void setPhase(float radians) { phase = wrapAngle(radians); }

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] vec3 getVelocity() const { return velocity; }
	[[nodiscard]] float getAngularVelocity() const { return angularVelocity; }
	[[nodiscard]] float getPhase() const { return phase; }

private:
	vec3 position = vec3();
	float angle = 0;
	vec3 velocity = vec3();
	float angularVelocity = 0;
	float phase = 0;
};


class IMotionSpec {
public:
	virtual ~IMotionSpec() = default;

	IMotionSpec(const IMotionSpec&) = delete;
	IMotionSpec& operator=(const IMotionSpec&) = delete;
	IMotionSpec(IMotionSpec&&) = delete;
	IMotionSpec& operator=(IMotionSpec&&) = delete;

	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec* shapeSpec) const;

	// Updates KinematicState by one physics frame. Purely incremental - KinematicState must be initialised separately.
	virtual void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const = 0;
	// Updates the (stationary) KinematicState for obstacles in the editor. Purely incremental - KinematicState must be initialised separately.
	virtual void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const = 0;

protected:
	IMotionSpec() = default;

private:
	virtual void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const = 0;
};

class StaticSpec : public IMotionSpec {
public:
	explicit StaticSpec(vec2 position, float angle) :
	    position(planarToWorld(position)),
	    angle(angle) {}

	~StaticSpec() override = default;

	void setAngle(float radians);

	void stepKinematicState(KinematicState&, const Smoother&) const override {}

	void updateEditorKinematicState(KinematicState&, const Smoother&) const override {}

private:
	vec3 position;
	float angle;
	mat3 rotation;

	void buildDomainMesh(std::vector<ObjectVertex>&, std::vector<Index>&, const AbstractShapeSpec*) const override {}
};

class TogglingPositionSpec : public IMotionSpec {
public:
	explicit TogglingPositionSpec(float angle, vec2 positionA, vec2 positionB) :
	    angle(angle),
	    positionA(planarToWorld(positionA)),
	    positionB(planarToWorld(positionB)) {
		setAngle(angle);
	}

	~TogglingPositionSpec() override = default;

	void setAngle(float radians);

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	float angle;
	mat3 rotation;
	vec3 positionA;
	vec3 positionB;

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] float getAngle() const { return angle; }
	[[nodiscard]] mat3 getRotation() const { return rotation; }
	[[nodiscard]] vec3 getPositionA() const { return positionA; }
	[[nodiscard]] vec3 getPositionB() const { return positionB; }
};

class TogglingAngleSpec : public IMotionSpec {
public:
	explicit TogglingAngleSpec(vec2 position, float angleA, float angleB) :
	    position(planarToWorld(position)),
	    angleA(angleA),
	    angleB(angleB) {}

	~TogglingAngleSpec() override = default;

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position;
	float angleA;
	float angleB;

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngleA() const { return angleA; }
	[[nodiscard]] float getAngleB() const { return angleB; }
};

class SpinningSpec : public IMotionSpec {
public:
	explicit SpinningSpec(vec2 position, float initialAngle, float angularVelocityA, float angularVelocityB) :
	    position(planarToWorld(position)),
	    initialAngle(initialAngle),
	    angularVelocityA(angularVelocityA),
	    angularVelocityB(angularVelocityB) {}

	~SpinningSpec() override = default;

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother&) const override;

private:
	vec3 position;
	float initialAngle;
	float angularVelocityA;
	float angularVelocityB;

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getInitialAngle() const { return initialAngle; }
	[[nodiscard]] float getAngularVelocityA() const { return angularVelocityA; }
	[[nodiscard]] float getAngularVelocityB() const { return angularVelocityB; }
};

class OscillatingPositionSpec : public IMotionSpec {
public:
	explicit OscillatingPositionSpec(float angle, vec2 position1, vec2 position2, float angularFrequencyA, float angularFrequencyB) :
		angle(angle),
		position1(planarToWorld(position1)),
		position2(planarToWorld(position2)),
		angularFrequencyA(angularFrequencyA),
		angularFrequencyB(angularFrequencyB) {}

	~OscillatingPositionSpec() override = default;

	void setAngle(float radians);

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position1;
	vec3 position2;
	float angle;
	mat3 rotation;
	float angularFrequencyA;
	float angularFrequencyB;

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
	explicit OscillatingAngleSpec(vec2 position, float angle1, float angle2, float angularFrequencyA, float angularFrequencyB) :
		position(planarToWorld(position)),
		angle1(angle1),
		angle2(angle2),
		angularFrequencyA(angularFrequencyA),
		angularFrequencyB(angularFrequencyB) {}
	
	~OscillatingAngleSpec() override = default;

	void stepKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

	void updateEditorKinematicState(KinematicState& kinematicState, const Smoother& smoother) const override;

private:
	vec3 position;
	float angle1;
	float angle2;
	float angularFrequencyA;
	float angularFrequencyB;

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec* shapeSpec) const override;

	[[nodiscard]] vec3 getPosition() const { return position; }
	[[nodiscard]] float getAngle1() const { return angle1; }
	[[nodiscard]] float getAngle2() const { return angle2; }
	[[nodiscard]] float getAngularFrequencyA() const { return angularFrequencyA; }
	[[nodiscard]] float getAngularFrequencyB() const { return angularFrequencyB; }
};


class ObstacleDescriptor {
public:
	ObstacleDescriptor(std::unique_ptr<IMotionSpec> motion, std::unique_ptr<AbstractShapeSpec> shape, bool goal = false) :
	    motion(std::move(motion)),
	    shape(std::move(shape)),
	    goal(goal) {}

	[[nodiscard]] IMotionSpec* getMotion() const { return motion.get(); }
	[[nodiscard]] AbstractShapeSpec* getShape() const { return shape.get(); }
	[[nodiscard]] bool isGoal() const { return goal; }

private:
	std::unique_ptr<IMotionSpec> motion;
	std::unique_ptr<AbstractShapeSpec> shape;
	bool goal;
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

	bool isSelected() const { return selected; };
	void select() { selected = true; };
	void deselect() { selected = false; };

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