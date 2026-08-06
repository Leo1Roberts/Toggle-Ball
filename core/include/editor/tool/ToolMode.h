#ifndef TOOL_MODE_H
#define TOOL_MODE_H

#include "io/Event.h"

struct EditorContext;


class ToolMode {
public:
	virtual ~ToolMode() = default;

	bool processEvent(const Event& event);

	virtual void renderGizmos() {}

protected:
	explicit ToolMode(const EditorContext* context)
		: context(context) {}

	const EditorContext* context;

	PointerEvent pointerDownEvent;

private:
	virtual bool doProcessEvent(const Event& event) { return false; }

	// Pointer down and up on the same spot
	virtual void performPrimaryAction(const PointerEvent& upEvent) = 0;
	virtual void performSecondaryAction(const PointerEvent& upEvent) {}
	virtual void startDrag(const PointerEvent& dragStartEvent) {}

	bool dragging = false;
	bool pointerPrimaryDown = false;
	bool pointerSecondaryDown = false;
};


#endif // TOOL_MODE_H
