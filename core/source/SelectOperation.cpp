#include "SelectOperation.h"

#include "EditorObstacle.h"
#include "GizmoRenderer.h"


void SelectOperation::renderGizmos() {
	context->gizmoRenderer->drawBox(box, {
		.fillColor = Color::SelectBox,
		.strokeColor = Color::Selected,
		.cornerRadius = 0.f,
		.strokeWidth = Settings::Sizes.outlineWidth,
	});
}


void SelectOperation::finish() const {
	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;
	auto focus = &context->scene->selectionFocus;

	if (instant && (mode == SelectionMode::Replace || mode == SelectionMode::Add)) {
		const auto& originalSelection = context->scene->getCurrentNode()->selectionNode->selectionState;
		bool revertSelection = false;

		if (ball->isInSelectBox(box)) {
			if (originalSelection.ball || mode == SelectionMode::Add)
				revertSelection = true;
			else {
				context->scene->deselectAll();
				ball->select();
			}
			*focus = {EntityType::Ball};
		} else {
			int topObstacleIndex = -1;
			float maxHalfDepth = 0.f;
			for (int i = 0; i < obstacles.size(); i++) {
				if (obstacles[i].isInSelectBox(box) && obstacles[i].descriptor->shape->getHalfDepth() >= maxHalfDepth) {
					maxHalfDepth = obstacles[i].descriptor->shape->getHalfDepth();
					topObstacleIndex = i;
				}
			}
			if (topObstacleIndex >= 0) {
				if (originalSelection.obstacles[topObstacleIndex] || mode == SelectionMode::Add)
					revertSelection = true;
				else {
					context->scene->deselectAll();
					obstacles[topObstacleIndex].select();
				}
				*focus = {EntityType::Obstacle, topObstacleIndex};
			} else if (mode == SelectionMode::Replace)
				*focus = {EntityType::None};
		}

		if (revertSelection) {
			ball->setSelected(originalSelection.ball || focus->type == EntityType::Ball);
			for (int i = 0; i < obstacles.size(); i++)
				obstacles[i].setSelected(originalSelection.obstacles[i] || (focus->type == EntityType::Obstacle && i == focus->index));
		}
	} else if (focus->type == EntityType::None ||
		      (focus->type == EntityType::Ball && !ball->isSelected()) ||
		      (focus->type == EntityType::Obstacle && !obstacles[focus->index].isSelected())) {
		*focus = {EntityType::None};
		if (ball->isSelected())
			*focus = {EntityType::Ball};
		else {
			for (int i = (int)obstacles.size() - 1; i >= 0; i--)
				if (obstacles[i].isSelected()) {
					*focus = {EntityType::Obstacle, i};
					break;
				}
		}
	}
}


void SelectOperation::applyModifiers(byte mods) {
	if (mods & MOD_SHIFT && !(mods & MOD_CTRL))
		mode = SelectionMode::Subtract;
	else if (mods & MOD_CTRL && !(mods & MOD_SHIFT))
		mode = SelectionMode::Add;
	else
		mode = SelectionMode::Replace;
}

bool SelectOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move) {
			glm::vec2 pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			box = {
				initialPointerPlanarPosition.x, pointerPlanarPosition.x,
				initialPointerPlanarPosition.y, pointerPlanarPosition.y,
			};
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	}
	return false;
}

void SelectOperation::applyOperation() {
	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;
	const auto& originalSelection = context->scene->getCurrentNode()->selectionNode->selectionState;

	switch (mode) {
	case SelectionMode::Replace:
		ball->setSelected(ball->isInSelectBox(box));
		for (auto& obstacle : obstacles)
			obstacle.setSelected(obstacle.isInSelectBox(box));
		break;
	case SelectionMode::Add:
		ball->setSelected(originalSelection.ball || ball->isInSelectBox(box));
		for (int i = 0; i < obstacles.size(); i++)
			obstacles[i].setSelected(originalSelection.obstacles[i] || obstacles[i].isInSelectBox(box));
		break;
	case SelectionMode::Subtract:
		ball->setSelected(originalSelection.ball && !ball->isInSelectBox(box));
		for (int i = 0; i < obstacles.size(); i++)
			obstacles[i].setSelected(originalSelection.obstacles[i] && !obstacles[i].isInSelectBox(box));
		break;
	}
}