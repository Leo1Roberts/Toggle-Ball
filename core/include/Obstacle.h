#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "Mesh.h"

class AbstractShapeSpec {
public:
	virtual ~AbstractShapeSpec() = default;

	void generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh) const;
	void generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh) const;

protected:
	explicit AbstractShapeSpec(float halfWidth) :
	    halfWidth(halfWidth) {}

	AbstractShapeSpec(const AbstractShapeSpec&) = default;
	AbstractShapeSpec& operator=(const AbstractShapeSpec&) = default;
	AbstractShapeSpec(AbstractShapeSpec&&) = default;
	AbstractShapeSpec& operator=(AbstractShapeSpec&&) = default;

private:
	float halfWidth;

	virtual void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;
	virtual void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const = 0;
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

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;
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

	void buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;
	void buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const override;
};


class IMotionSpec {
public:
	virtual ~IMotionSpec() = default;

	IMotionSpec(const IMotionSpec&) = delete;
	IMotionSpec& operator=(const IMotionSpec&) = delete;
	IMotionSpec(IMotionSpec&&) = delete;
	IMotionSpec& operator=(IMotionSpec&&) = delete;

	void generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec& shapeSpec) const;

protected:
	IMotionSpec() = default;

private:
	virtual void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {}
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

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const override;
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

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const override;
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

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const override;
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

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const override;
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

	void buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const override;
};


struct KinematicState {
	vec2 position = vec2();
	float angle = 0;
	vec2 velocity = vec2();
	float angularVelocity = 0;
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
	    descriptor(descriptor) {
		assert(descriptor);
	}

	~EditorObstacle() = default;

	EditorObstacle(const EditorObstacle& other) = delete;
	EditorObstacle& operator=(const EditorObstacle&) = delete;
	EditorObstacle(EditorObstacle&&) = default;
	EditorObstacle& operator=(EditorObstacle&&) = default;

	bool isSelected() { return selected; };
	void select() { selected = true; };
	void deselect() { selected = false; };

private:
	ObstacleDescriptor* descriptor;

	/*
	 * Needed for obstacles with 'memory':
	 * Kinematic state cannot be calculated using only the motion spec and smoother
	 * Spinning obstacles - angle
	 * Oscillating obstacles - phase
	 */
	float phase = 0;

	bool selected = false;

	Mesh<ObjectVertex> obstacle = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> outline = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> domain = Mesh<ObjectVertex>(GL_STATIC_DRAW);
};

#endif // OBSTACLE_H