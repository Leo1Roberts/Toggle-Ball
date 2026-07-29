#ifndef OPERATION_H
#define OPERATION_H

#include <utility>

#include "EditorContext.h"
#include "Event.h"


enum class TriggerType {
	PointerPrimary,
	PointerSecondary,
	Key,
};

struct EditorContext;
class Operation {
public:
	virtual ~Operation() = default;

	bool processEvent(const Event& event);

	virtual void renderGizmos() {}

	void cancel() const {
		doCancel();
		context.finishOperation();
	}
	void commit() const {
		doCommit();
		context.finishOperation();
	}

protected:
	Operation(EditorContext context, TriggerType trigger, glm::vec2 initialPointerPosition)
		: context(std::move(context)), trigger(trigger), initialPointerPosition(initialPointerPosition) {}

	EditorContext context;
	TriggerType trigger{};
	glm::vec2 initialPointerPosition{};

private:
	virtual void applyModifiers(byte mods) = 0;
	virtual bool doProcessEvent(const Event& event) = 0;
	virtual void applyOperation() = 0;

	virtual void doCancel() const = 0;
	virtual void doCommit() const = 0;
};


#endif // OPERATION_H
