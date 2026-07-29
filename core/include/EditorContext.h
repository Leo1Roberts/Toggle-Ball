#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include <functional>
#include <memory>


class Operation;
class GizmoRenderer;
class Camera;
class EditorScene;

struct EditorContext {
	EditorScene* scene;
	const Camera* camera;
	GizmoRenderer* gizmoRenderer;

	std::function <Operation*(std::unique_ptr<Operation>)> startOperation;
	std::function <void()> finishOperation;
};


#endif // EDITOR_CONTEXT_H
