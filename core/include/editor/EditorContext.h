#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include <functional>
#include <memory>


class Operation;
class GizmoRenderer;
class Camera;
class EditorScene;
class UINode;

struct EditorQuickSettings {
	bool transformBothStates = false;
	bool transformLocally = false;
};

struct EditorContext {
	EditorQuickSettings* quickSettings;
	EditorScene* scene;
	const Camera* camera;
	UINode* operationUI;
	GizmoRenderer* gizmoRenderer;
	const float* uiToWorldScale;

	std::function <Operation*(std::unique_ptr<Operation>)> startOperation;
	std::function <void()> finishOperation;
};


#endif // EDITOR_CONTEXT_H
