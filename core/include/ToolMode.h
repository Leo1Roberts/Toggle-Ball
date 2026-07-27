#ifndef TOOL_MODE_H
#define TOOL_MODE_H


#include "Event.h"

struct EditorContext;

class ToolMode {
public:
	virtual ~ToolMode() = default;

	bool processEvent(const Event& event, EditorContext& editor);

private:
	// Pointer down and up on the same spot
	virtual void performPrimaryAction(EditorContext& editor, const PointerEvent& upEvent) = 0;
	virtual void performSecondaryAction(EditorContext& editor, const PointerEvent& upEvent) = 0;
	// Pointer down and started to move
	virtual void startPrimaryDrag(EditorContext& editor, glm::vec2 pointerDownPosition, const PointerEvent& moveEvent) = 0;
	virtual void startSecondaryDrag(EditorContext& editor, glm::vec2 pointerDownPosition, const PointerEvent& moveEvent) = 0;

	bool pointerPrimaryDown = false;
	bool pointerSecondaryDown = false;
	glm::vec2 pointerDownPosition{};
};


#endif // TOOL_MODE_H
