#ifndef EDITOR_CONTEXT_H
#define EDITOR_CONTEXT_H

#include "io/KeyBindings.h"

#include <functional>
#include <memory>


class Camera;
class EditorScene;
class GizmoRenderer;
class Operation;
class UIHorizontalList;
class UINode;

struct EditorQuickSettings {
	bool transformBothStates = false;
	bool transformIndividually = false;
};

struct EditorContext {
	EditorQuickSettings* quickSettings;
	EditorScene* scene;
	const Camera* camera;
	GizmoRenderer* gizmoRenderer;
	const float* uiToWorldScale;

	UINode* operationUI;
	UIHorizontalList* operationShortcutHints;

	std::function <Operation*(std::unique_ptr<Operation>)> startOperation;
	std::function <void()> finishOperation;

	static std::unique_ptr<UIHorizontalList> makeShortcutHint(KeyChord keyChord, const std::string& effect);
};


#endif // EDITOR_CONTEXT_H
