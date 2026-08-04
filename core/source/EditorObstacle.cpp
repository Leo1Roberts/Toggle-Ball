#include "EditorObstacle.h"


void EditorObstacle::updateKinematicState(const Smoother& smoother, int numSteps) {
	if (numSteps < 0) // Just set 'stationary' attributes
		descriptor->motion->updateEditorKinematicState(kinematicState, smoother);
	else // Demonstrate motion
		while (numSteps--)
			descriptor->motion->stepKinematicState(kinematicState, smoother);
}