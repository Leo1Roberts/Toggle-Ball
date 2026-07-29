#ifndef TOOL_MODE_H
#define TOOL_MODE_H


#include "EditorContext.h"
#include "Event.h"
#include "Operation.h"

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

	// Pointer down and started to move
	void startPrimaryDrag(const PointerEvent& moveEvent) {
		if (auto operation = startPrimaryDragOperation())
			operation->processEvent(moveEvent);
	}
	void startSecondaryDrag(const PointerEvent& moveEvent) {
		if (auto operation = startSecondaryDragOperation())
			operation->processEvent(moveEvent);
	}
	virtual Operation* startPrimaryDragOperation() { return nullptr; }
	virtual Operation* startSecondaryDragOperation() { return nullptr; }

	bool pointerPrimaryDown = false;
	bool pointerSecondaryDown = false;
};


#endif // TOOL_MODE_H
