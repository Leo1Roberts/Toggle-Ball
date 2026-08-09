#ifndef OPERATION_H
#define OPERATION_H

#include "utilities/Camera.h"
#include "editor/EditorContext.h"
#include "io/Event.h"


enum class TriggerType {
	Pointer,
	TriggerKey,
	ActionKey,
};

class Operation {
public:
	virtual ~Operation() = default;

	bool processEvent(const Event& event);

	virtual void renderGizmos() {}

	[[nodiscard]] bool start(byte mods) {
		if (canStart()) {
			applyModifiers(mods);
			applyOperation();
			return true;
		}
		return false;
	}
	void cancel() const {
		doCancel();
		context->finishOperation();
	}
	virtual void finish() const {}
	void commit() const {
		doCommit();
		context->finishOperation();
	}

	void onQuickSettingsChanged() { applyOperation(); }

protected:
	Operation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: context(context), trigger(trigger), initialPointerPlanarPosition(context->camera->screenToPlanarPosition(initialPointerPosition)) {}

	const EditorContext* context;
	TriggerType trigger{};
	glm::vec2 initialPointerPlanarPosition{};

private:
	virtual void applyModifiers(byte mods) = 0;
	virtual bool doProcessEvent(const Event& event) = 0;
	virtual void applyOperation() = 0;

	[[nodiscard]] virtual bool canStart() const { return true; }
	virtual void doCancel() const = 0;
	virtual void doCommit() const = 0;
};


#endif // OPERATION_H
