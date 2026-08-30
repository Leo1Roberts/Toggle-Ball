#include "editor/operation/Operation.h"

#include "editor/EditorScene.h"


OperationResponse Operation::processEvent(const Event& event) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (key->chord.code == KeyCode::Escape) {
				cancel();
				return {true, OperationStatus::Cancelled};
			}
			if (key->chord.code == KeyCode::Enter) {
				finish();
				commit();
				return {true, OperationStatus::Committed};
			}
		}
		switch (key->chord.code) {
		case KeyCode::Ctrl:
		case KeyCode::Shift:
		case KeyCode::Alt:
			applyModifiers(key->chord.modifiers);
			applyOperation();
		default:;
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::FinishDrag:
			finish();
			commit();
			return {true, OperationStatus::Committed};
		case PointerAction::CancelDrag:
			cancel();
			return {true, OperationStatus::Cancelled};
		case PointerAction::Down:
			if (trigger == TriggerType::TriggerKey || trigger == TriggerType::ActionKey) {
				if (pointer->button == PointerButton::Primary) {
					finish();
					commit();
					return {trigger == TriggerType::TriggerKey, OperationStatus::Committed};
				}
				if (pointer->button == PointerButton::Secondary) {
					cancel();
					return {trigger == TriggerType::TriggerKey, OperationStatus::Cancelled};
				}
			}
			break;
		default:;
		}
	}

	return doProcessEvent(event);
}


std::optional<int> Operation::getTopObstacleIndex(const std::vector<EditorObstacle>& obstacles, const std::function<bool(const EditorObstacle&)>& includePredicate, bool prioritiseSelected) {
	if (prioritiseSelected) {
		int topSelectedObstacleIndex = -1;
		float maxSelectedHalfDepth = 0.f;

		for (int i = 0; i < obstacles.size(); i++) {
			const auto& obstacle = obstacles[i];
			if (obstacle.isSelected() && includePredicate(obstacle) &&
				obstacle.descriptor->shape->getHalfDepth() >= maxSelectedHalfDepth) {
				maxSelectedHalfDepth = obstacle.descriptor->shape->getHalfDepth();
				topSelectedObstacleIndex = i;
			}
		}

		if (topSelectedObstacleIndex >= 0)
			return topSelectedObstacleIndex;
	}

	int topObstacleIndex = -1;
	float maxHalfDepth = 0.f;
	for (int i = 0; i < obstacles.size(); i++) {
		const auto& obstacle = obstacles[i];
		if (includePredicate(obstacle) &&
			(obstacle.descriptor->shape->getHalfDepth() > maxHalfDepth ||
			(obstacle.descriptor->shape->getHalfDepth() == maxHalfDepth && obstacle.isSelected()))) {
			maxHalfDepth = obstacle.descriptor->shape->getHalfDepth();
			topObstacleIndex = i;
		}
	}

	if (topObstacleIndex >= 0)
		return topObstacleIndex;

	return std::nullopt;
}
std::optional<int> Operation::getPointedObstacleIndex(const std::vector<EditorObstacle>& obstacles, glm::vec2 pointerPlanarPosition, bool prioritiseSelected) {
	return getTopObstacleIndex(obstacles, [pointerPlanarPosition](const EditorObstacle& obstacle) {
		return obstacle.isInSelectBox(SelectBox(pointerPlanarPosition));
	}, prioritiseSelected);
}