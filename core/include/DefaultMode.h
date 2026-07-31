#ifndef DEFAULT_MODE_H
#define DEFAULT_MODE_H

#include "ToolMode.h"


class DefaultMode : public ToolMode {
public:
	explicit DefaultMode(const EditorContext* context)
		: ToolMode(context) {}

private:
	bool doProcessEvent(const Event& event) override;

	void performPrimaryAction(const PointerEvent& upEvent) override;

	Operation* startPrimaryDragOperation() override;
};


#endif // DEFAULT_MODE_H
