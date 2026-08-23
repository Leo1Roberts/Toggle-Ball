#include "editor/EditorObstacle.h"


void EditorObstacle::updateKinematicState(const Smoother& smoother, int numSteps) {
	if (numSteps < 0) // Just set 'stationary' attributes
		descriptor->motion->updateEditorKinematicState(kinematicState, smoother);
	else // Demonstrate motion
		while (numSteps--)
			descriptor->motion->stepKinematicState(kinematicState, smoother);
}


const Mesh<ObjectVertex>* EditorObstacle::getObstacleMesh() {
	if (!obstacleMeshValid) {
		descriptor->generateObstacleMesh(obstacleMesh);
		obstacleMeshValid = true;
	}
	return &obstacleMesh;
}
const Mesh<ObjectVertex>* EditorObstacle::getOutlineMesh(float uiToWorldScale) {
	if (!outlineMeshValid) {
		descriptor->generateOutlineMesh(outlineMesh, uiToWorldScale);
		outlineMeshValid = true;
	}
	return &outlineMesh;
}
const Mesh<ObjectVertex>* EditorObstacle::getDomainMesh(float uiToWorldScale) {
	if (!domainMeshValid) {
		descriptor->generateDomainMesh(domainMesh, uiToWorldScale);
		domainMeshValid = true;
	}
	return &domainMesh;
}