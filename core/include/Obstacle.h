#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "Model.h"

class IMotionSpec {
public:
	virtual ~IMotionSpec() = default;

	IMotionSpec(const IMotionSpec&) = delete;
	IMotionSpec& operator=(const IMotionSpec&) = delete;
	IMotionSpec(IMotionSpec&&) = delete;
	IMotionSpec& operator=(IMotionSpec&&) = delete;

protected:
	IMotionSpec() = default;
};

class StaticSpec : public IMotionSpec {
public:
	explicit StaticSpec(vec2 position, float angle) :
	    position(position),
	    angle(angle) {}

	~StaticSpec() override = default;

private:
	vec2 position;
	float angle;
};

class SpinningSpec : public IMotionSpec {
public:
	explicit SpinningSpec(vec2 position, float initialAngle, float angularVelocityA, float angularVelocityB) :
	    position(position),
	    initialAngle(initialAngle),
	    angularVelocityA(angularVelocityA),
	    angularVelocityB(angularVelocityB) {}

	~SpinningSpec() override = default;

private:
	vec2 position;
	float initialAngle;

	float angularVelocityA;
	float angularVelocityB;
};

class TogglingPositionSpec : public IMotionSpec {
public:
	explicit TogglingPositionSpec(float angle, vec2 positionA, vec2 positionB) :
	    angle(angle),
	    positionA(positionA),
	    positionB(positionB) {}

	~TogglingPositionSpec() override = default;

private:
	float angle;

	vec2 positionA;
	vec2 positionB;
};

class TogglingAngleSpec : public IMotionSpec {
public:
	explicit TogglingAngleSpec(vec2 position, float angleA, float angleB) :
	    position(position),
	    angleA(angleA),
	    angleB(angleB) {}

	~TogglingAngleSpec() override = default;

private:
	vec2 position;

	float angleA;
	float angleB;
};

class OscillatingPositionSpec : public IMotionSpec {
public:
	explicit OscillatingPositionSpec(float angle, vec2 position1, vec2 position2, float angularFrequencyA, float angularFrequencyB) :
		angle(angle),
		position1(position1),
		position2(position2),
		angularFrequencyA(angularFrequencyA),
		angularFrequencyB(angularFrequencyB) {}

	~OscillatingPositionSpec() override = default;

private:
	float angle;
	vec2 position1;
	vec2 position2;

	float angularFrequencyA;
	float angularFrequencyB;
};

class OscillatingAngleSpec : public IMotionSpec {
public:
	explicit OscillatingAngleSpec(vec2 position, float angle1, float angle2, float angularFrequencyA, float angularFrequencyB) :
		position(position),
		angle1(angle1),
		angle2(angle2),
		angularFrequencyA(angularFrequencyA),
		angularFrequencyB(angularFrequencyB) {}
	
	~OscillatingAngleSpec() override = default;

private:
	vec2 position;
	float angle1;
	float angle2;

	float angularFrequencyA;
	float angularFrequencyB;
};


struct KinematicState {
	vec2 position;
	float angle;
	vec2 velocity;
	float angularVelocity;
};


class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

protected:
	explicit AbstractShapeSpec(float halfWidth) :
	    halfWidth(halfWidth) {}

	AbstractShapeSpec(const AbstractShapeSpec&) = default;
	AbstractShapeSpec& operator=(const AbstractShapeSpec&) = default;
	AbstractShapeSpec(AbstractShapeSpec&&) = default;
	AbstractShapeSpec& operator=(AbstractShapeSpec&&) = default;

private:
	float halfWidth;
};

class SegmentSpec : public AbstractShapeSpec {
public:
	SegmentSpec(float halfWidth, float left, float right) :
	    AbstractShapeSpec(halfWidth),
	    left(left),
	    right(right) {}

	~SegmentSpec() override = default;

private:
	float left;
	float right;
};

class ArcSpec : public AbstractShapeSpec {
public:
	ArcSpec(float halfWidth, float angle, float radius) :
	    AbstractShapeSpec(halfWidth),
	    angle(angle),
	    radius(radius) {}

	~ArcSpec() override = default;

private:
	float angle;
	float radius;
};


class ObstacleDescriptor {
public:
	ObstacleDescriptor(bool isGoal, std::unique_ptr<IMotionSpec> motion, std::unique_ptr<AbstractShapeSpec> shape) :
	    isGoal(isGoal),
	    motion(std::move(motion)),
	    shape(std::move(shape)) {}

private:
	bool isGoal;
	std::unique_ptr<IMotionSpec> motion;
	std::unique_ptr<AbstractShapeSpec> shape;
};


class EditorObstacle {
public:
	explicit EditorObstacle(ObstacleDescriptor* descriptor) :
	    descriptor(descriptor),
		phase(0) {
		assert(descriptor != nullptr);
	}

	~EditorObstacle() = default;

	EditorObstacle(const EditorObstacle& other) = delete;
	EditorObstacle& operator=(const EditorObstacle&) = delete;
	EditorObstacle(EditorObstacle&&) = default;
	EditorObstacle& operator=(EditorObstacle&&) = default;

private:
	ObstacleDescriptor* descriptor;

	/*
	 * Needed for obstacles with 'memory':
	 * Kinematic state cannot be calculated using only the motion spec and smoother
	 * Spinning obstacles: angle
	 * Oscillating obstacles: phase
	 */
	float phase;

	Model obstacle;
	Model outline;
	Model domain;
};

#endif // OBSTACLE_H