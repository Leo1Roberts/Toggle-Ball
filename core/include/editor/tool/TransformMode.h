#ifndef DEFAULT_MODE_H
#define DEFAULT_MODE_H

#include "ToolMode.h"


class TransformMode : public ToolMode {
public:
	explicit TransformMode(const EditorContext* context)
		: ToolMode(context) {}

private:
	bool doProcessEvent(const Event& event) override;

	void performPrimaryAction(const PointerEvent& upEvent) override;
	void startDrag(const PointerEvent& dragStartEvent) override;
};


#endif // DEFAULT_MODE_H
