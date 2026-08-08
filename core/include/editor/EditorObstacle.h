#ifndef EDITOR_OBSTACLE_H
#define EDITOR_OBSTACLE_H

#include "opengl/Mesh.h"
#include "obstacle/ObstacleDescriptor.h"
#include "obstacle/ObstacleKinematicState.h"
#include "SelectBox.h"
#include "utilities/Utilities.h"

#include <glm/glm.hpp>


class Smoother;

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

	void setMotionProperty(float value, MotionSpecProperty property, bool toggled = false) {
		descriptor->motion->setProperty(value, property, toggled);
	}
	[[nodiscard]] float getMotionProperty(MotionSpecProperty property, bool toggled = false) const {
		return descriptor->motion->getProperty(property, toggled);
	}

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }
	void setSelected(bool select) { selected = select; }
	[[nodiscard]] bool isInSelectBox(SelectBox box) const { return descriptor->shape->isInSelectBox(kinematicState, box); }

	[[nodiscard]] const ObstacleKinematicState* getKinematicState() const { return &kinematicState; }
	[[nodiscard]] const Mesh<ObjectVertex>* getObstacleMesh() const { return &obstacleMesh; }
	[[nodiscard]] const Mesh<ObjectVertex>* getOutlineMesh() const { return &outlineMesh; }
	[[nodiscard]] const Mesh<ObjectVertex>* getDomainMesh() const { return &domainMesh; }

	[[nodiscard]] glm::vec2 getDomainPosition() const { return descriptor->getDomainPosition(worldToPlanar(kinematicState.getPosition())); }

	ObstacleDescriptor* descriptor;

private:
	ObstacleKinematicState kinematicState;

	bool selected = false;

	Mesh<ObjectVertex> obstacleMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> outlineMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> domainMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
};


#endif // EDITOR_OBSTACLE_H
