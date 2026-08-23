#include "editor/tool/ToolMode.h"

#include "editor/EditorScene.h"
#include "editor/SelectBox.h"
#include "editor/operation/SelectOperation.h"
#include "editor/operation/TranslateOperation.h"


ToolModeResponse ToolMode::processEvent(const Event& event) {
	bool operationChanged = false;

	if (activeOperation) {
		auto response = activeOperation->processEvent(event);
		if (response.status != OperationStatus::Running) {
			activeOperation.reset();
			operationChanged = true;
		}
		if (response.consumedEvent)
			return {.consumedEvent = true, .operationChanged = operationChanged};
	}

	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->id == 0)
			pointer0Position = pointer->position;

		switch (pointer->action) {
		case PointerAction::Down:
			pointerDownEvent = *pointer;
			if (pointer->button == PointerButton::Primary)
				pointerPrimaryDown = true;
			else if (pointer->button == PointerButton::Secondary)
				pointerSecondaryDown = true;
			break;
		case PointerAction::StartDrag:
			dragging = true;
			if (pointer->button == PointerButton::Primary && pointerPrimaryDown ||
				pointer->button == PointerButton::Secondary && pointerSecondaryDown) {
				activeOperation = startDrag(*pointer);
				return {.consumedEvent = true, .operationChanged = activeOperation != nullptr};
			}
			return {.consumedEvent = true, .operationChanged = false};
		case PointerAction::Up:
			if (dragging) {
				if (pointer->button == PointerButton::Primary)
					pointerPrimaryDown = false;
				else if (pointer->button == PointerButton::Secondary)
					pointerSecondaryDown = false;
				if (!pointerPrimaryDown && !pointerSecondaryDown)
					dragging = false;
			} else {
				if (pointer->button == PointerButton::Primary) {
					bool doPrimaryAction = pointerPrimaryDown && !pointerSecondaryDown;
					pointerPrimaryDown = false;
					if (doPrimaryAction) {
						performPrimaryAction(*pointer);
						return {.consumedEvent = true, .operationChanged = false};
					}
				} else if (pointer->button == PointerButton::Secondary) {
					bool doSecondaryAction = pointerSecondaryDown && !pointerPrimaryDown;
					pointerSecondaryDown = false;
					if (doSecondaryAction) {
						performSecondaryAction(*pointer);
						return {.consumedEvent = true, .operationChanged = false};
					}
				}
			}
			break;
		default:;
		}
	}

	auto response = doProcessEvent(event);
	response.operationChanged |= operationChanged;
	return response;
}

ToolModeResponse ToolMode::processObstacleExistenceAction(ActionCode actionCode, byte modifiers, const TransformQuickSettings& settings) {
	switch (actionCode) {
	case ActionCode::Copy:
		scene->copySelection();
		return {.consumedEvent = true, .operationChanged = false};
	case ActionCode::Delete: {
		bool operationChanged = false;
		if (activeOperation) {
			auto ss = scene->getSelectionState();
			activeOperation->cancel();
			activeOperation.reset();
			operationChanged = true;
			scene->applySelectionState(ss);
		}
		scene->deleteSelection();
		return {.consumedEvent = true, .operationChanged = operationChanged};
	}
	case ActionCode::Cut: {
		scene->copySelection();
		bool operationChanged = false;
		if (activeOperation) {
			auto ss = scene->getSelectionState();
			activeOperation->cancel();
			activeOperation.reset();
			operationChanged = true;
			scene->applySelectionState(ss);
		}
		scene->deleteSelection();
		return {.consumedEvent = true, .operationChanged = operationChanged};
	}
	case ActionCode::Paste:
		if (!activeOperation && scene->paste()) {
			auto meanCentre = glm::vec2(0.f);
			int selectedCount = 0;
			for (const auto& obstacle : scene->obstacles)
				if (obstacle.isSelected()) {
					meanCentre += worldToPlanar(obstacle.getKinematicState()->getPosition());
					selectedCount++;
				}
			meanCentre /= selectedCount;

			activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::TriggerKey, meanCentre);
			if (!activeOperation->start(modifiers) ||
				activeOperation->processEvent(PointerEvent(0, pointer0Position, PointerAction::Move)).status != OperationStatus::Running)
				activeOperation.reset();
			return {.consumedEvent = true, .operationChanged = true};
		}
		break;
	case ActionCode::Duplicate:
		if (!activeOperation && scene->duplicateSelection()) {
			activeOperation = std::make_unique<TranslateOperation>(scene, camera, settings, TriggerType::TriggerKey, camera->screenToPlanarPosition(pointer0Position));
			if (!activeOperation->start(modifiers))
				activeOperation.reset();
			return {.consumedEvent = true, .operationChanged = true};
		}
		break;
	default:;
	}

	return {.consumedEvent = false, .operationChanged = false};
}


void ToolMode::createOperationUI(UINode& container) const {
	if (activeOperation) {
		activeOperation->createUI(container);
		activeOperation->updateUI();
	}
}


void ToolMode::cancelActiveOperation() {
	if (activeOperation)
		activeOperation->cancel();
	activeOperation.reset();
}
void ToolMode::commitActiveOperation() {
	if (activeOperation) {
		activeOperation->finish();
		activeOperation->commit();
	}
	activeOperation.reset();
}


bool ToolMode::pointedAtBall(glm::vec2 pointerPosition) const {
	auto hitTestBox = SelectBox(camera->screenToPlanarPosition(pointerPosition));
	return scene->ball.isInSelectBox(hitTestBox);
}
bool ToolMode::pointedAtObstacle(glm::vec2 pointerPosition) const {
	auto hitTestBox = SelectBox(camera->screenToPlanarPosition(pointerPosition));
	return std::ranges::any_of(scene->obstacles,
		[&hitTestBox](const auto& obstacle) {
			return obstacle.isInSelectBox(hitTestBox);
		});
}


void ToolMode::performPrimaryAction(const PointerEvent& upEvent) {
	auto selectOperation = SelectOperation(
		scene, camera, TriggerType::Pointer,
		camera->screenToPlanarPosition(pointerDownEvent.position), true);
	if (selectOperation.start(pointerDownEvent.modifiers)) {
		selectOperation.finish();
		selectOperation.commit();
	}
}