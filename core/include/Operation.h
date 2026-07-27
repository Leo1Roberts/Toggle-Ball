#ifndef OPERATION_H
#define OPERATION_H


#include "Event.h"


enum class TriggerType {
	Drag,
	Key,
};

struct EditorContext;
class Operation {
public:
	virtual ~Operation() = default;

	bool processEvent(const Event& event, EditorContext& editor);

protected:
	TriggerType trigger{};

private:
	virtual bool doProcessEvent(const Event& event, EditorContext& editor) = 0;
};


#endif // OPERATION_H
