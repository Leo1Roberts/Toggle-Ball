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
	}

	~EditorObstacle() = default;

	EditorObstacle(const EditorObstacle& other) = delete;
	EditorObstacle& operator=(const EditorObstacle&) = delete;
	EditorObstacle(EditorObstacle&&) = default;
	EditorObstacle& operator=(EditorObstacle&&) = default;

	void changeMotion(IMotionSpec::Type type, bool toggled) {
		descriptor->changeMotion(type, motionPropertyValues, toggled);
		initKinematicState();
		invalidateAllMeshes();
	}
	void setIsGoal(bool isGoal) {
		descriptor->setIsGoal(isGoal);
		invalidateAllMeshes();
	}

	void invalidateAllMeshes() {
		obstacleMeshValid = outlineMeshValid = domainMeshValid = false;
	}
	void invalidateEphemeralMeshes() {
		outlineMeshValid = domainMeshValid = false;
	}
	void invalidateDomainMesh() {
		domainMeshValid = false;
	}

	void initKinematicState(bool keepPhase = false) { descriptor->motion->initKinematicState(kinematicState, keepPhase); }
	// Only provide numSteps if demonstrating the continuous motion of an obstacle
	void updateKinematicState(const Smoother& smoother, int numSteps = -1);

	void translateBy(glm::vec2 vector, bool stateless, bool toggled, const ObstacleDescriptor* base) const {
		descriptor->motion->translateBy(vector, stateless, toggled, base->motion.get());
	}
	void rotateBy(float radians, glm::mat2 rotationMatrix, glm::vec2 pivot, bool stateless, bool toggled, bool individual, const ObstacleDescriptor* base) const {
		descriptor->motion->rotateBy(radians, rotationMatrix, pivot, stateless, toggled, individual, base->motion.get());
	}
	void scaleBy(float factor, glm::vec2 pivot, bool individual,
		bool affectMinorRadius, bool affectMajorRadius,
		const ObstacleDescriptor* base) const {
		descriptor->motion->scaleBy(factor, pivot, individual, base->motion.get());
		descriptor->shape->scaleBy(factor, affectMinorRadius, affectMajorRadius, base->shape.get());
	}

	void setMotionProperty(float value, IMotionSpec::PropertyDescriptor property) {
		descriptor->motion->setProperty(value, property);
		initKinematicState(true);
		invalidateAllMeshes();
	}
	[[nodiscard]] std::optional<float> getMotionProperty(IMotionSpec::PropertyDescriptor property) const {
		return descriptor->motion->getProperty(true, property);
	}

	[[nodiscard]] bool isSelected() const { return selected; }
	void select() { selected = true; }
	void deselect() { selected = false; }
	void setSelected(bool select) { selected = select; }
	[[nodiscard]] bool isInSelectBox(SelectBox box) const { return descriptor->shape->isInSelectBox(kinematicState, box); }

	[[nodiscard]] const ObstacleKinematicState* getKinematicState() const { return &kinematicState; }

	[[nodiscard]] const Mesh<ObjectVertex>* getObstacleMesh();
	[[nodiscard]] const Mesh<ObjectVertex>* getOutlineMesh(float uiToWorldScale);
	[[nodiscard]] const Mesh<ObjectVertex>* getDomainMesh(float uiToWorldScale);

	[[nodiscard]] glm::vec2 getDomainPosition() const { return descriptor->getDomainPosition(worldToPlanar(kinematicState.getPosition())); }

	ObstacleDescriptor* descriptor;

private:
	IMotionSpec::IncompletePropertyValues motionPropertyValues{std::nullopt};

	ObstacleKinematicState kinematicState;

	bool selected = true;

	Mesh<ObjectVertex> obstacleMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> outlineMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	Mesh<ObjectVertex> domainMesh = Mesh<ObjectVertex>(GL_STATIC_DRAW);
	bool obstacleMeshValid = false, outlineMeshValid = false, domainMeshValid = false;
};


#endif // EDITOR_OBSTACLE_H
