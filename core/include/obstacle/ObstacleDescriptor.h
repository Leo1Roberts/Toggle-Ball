#ifndef OBSTACLE_DESCRIPTOR_H
#define OBSTACLE_DESCRIPTOR_H

#include "opengl/Mesh.h"
#include "ObstacleMotion.h"
#include "ObstacleShape.h"
#include "game/PhysicsConstants.h"


struct ObstacleDescriptor {
	ObstacleDescriptor(std::unique_ptr<AbstractShapeSpec> shape, std::unique_ptr<IMotionSpec> motion, bool goal = false) :
		shape(std::move(shape)),
		motion(std::move(motion)),
		goal(goal),
		material(MAT_CONCRETE) {
		updateColor();
	}

	ObstacleDescriptor(const ObstacleDescriptor& other);
	ObstacleDescriptor& operator=(const ObstacleDescriptor& other);

	ObstacleDescriptor(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	bool operator==(const ObstacleDescriptor& other) const;

	void scale(float factor);

	void changeMotion(IMotionSpec::Type type, IMotionSpec::IncompletePropertyValues& values, bool toggled);

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

private:
	void updateColor() {
		color = goal ? Color::SoftGreen : this->motion->getColor();
	}
};


#endif // OBSTACLE_DESCRIPTOR_H
