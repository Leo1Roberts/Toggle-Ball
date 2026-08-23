#include "editor/tool/ShapeMode.h"

#include "editor/operation/DrawOperation.h"


ToolModeResponse ShapeMode::doProcessEvent(const Event& event) {
	if (auto key = std::get_if<KeyEvent>(&event))
		if (auto actionCode = Settings::Bindings->translate(key->chord))
			if (key->action == KeyAction::Down)
				return processObstacleExistenceAction(*actionCode, key->chord.modifiers);

	return {.consumedEvent = false, .operationChanged = false};
}


std::unique_ptr<Operation> ShapeMode::startDrag(const PointerEvent& dragStartEvent) {
	if (dragStartEvent.button == PointerButton::Primary) {
		auto drawOperation = std::make_unique<DrawOperation>(scene, camera, TriggerType::Pointer,
			camera->screenToPlanarPosition(pointerDownEvent.position), minorRadius);
		if (drawOperation->start(pointerDownEvent.modifiers))
			return drawOperation;
	}

	return nullptr;
}