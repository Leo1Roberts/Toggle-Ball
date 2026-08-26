#include "editor/operation/SelectOperation.h"

#include "editor/EditorObstacle.h"
#include "editor/GizmoRenderer.h"


std::vector<BindingHint> SelectOperation::getBindingHints() const {
	return {
		{KeyChord(KeyCode::Unknown, MOD_CTRL), "Add to selection"},
		{KeyChord(KeyCode::Unknown, MOD_SHIFT), "Remove from selection"},
	};
}

void SelectOperation::renderGizmos(GizmoRenderer& gizmoRenderer) {
	gizmoRenderer.addBox(box, {
		.fillColor = Color::SelectBox,
		.strokeColor = Color::Selected,
		.cornerRadius = 0.f,
		.strokeWidth = Settings::Sizes.outlineWidth,
	});

	gizmoRenderer.render();
}


void SelectOperation::finish() {
	auto ball = &scene.ball;
	auto& obstacles = scene.obstacles;
	auto focus = &scene.selectionFocus;

	if (instant && (mode == SelectionMode::Replace || mode == SelectionMode::Add)) {
		const auto& originalSelection = scene.getCurrentNode()->selectionNode->selectionState;
		bool revertSelection = false;

		if (ball->isInSelectBox(box)) {
			if (originalSelection.ball || mode == SelectionMode::Add)
				revertSelection = true;
			else {
				scene.deselectAll();
				ball->select();
			}
			*focus = {EntityType::Ball};
		} else {
			if (auto index = getTopObstacleIndex(scene.obstacles, [this](const EditorObstacle& obstacle) { return obstacle.isInSelectBox(box); })) {
				if (originalSelection.obstacles[*index] || mode == SelectionMode::Add)
					revertSelection = true;
				else {
					scene.deselectAll();
					obstacles[*index].select();
				}
				*focus = {EntityType::Obstacle, *index};
			} else if (mode == SelectionMode::Replace)
				*focus = {};
		}

		if (revertSelection) {
			ball->setSelected(originalSelection.ball || focus->type == EntityType::Ball);
			for (int i = 0; i < obstacles.size(); i++)
				obstacles[i].setSelected(originalSelection.obstacles[i] || (focus->type == EntityType::Obstacle && i == focus->index));
		}
	} else if (focus->type == EntityType::None ||
		      (focus->type == EntityType::Ball && !ball->isSelected()) ||
		      (focus->type == EntityType::Obstacle && !obstacles[focus->index].isSelected())) {
		*focus = {};
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

OperationResponse SelectOperation::doProcessEvent(const Event& event) {
	if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag) {
			glm::vec2 pointerPlanarPosition = camera.screenToPlanarPosition(pointer->position);
			box = {
				initialPlanarPosition.x, pointerPlanarPosition.x,
				initialPlanarPosition.y, pointerPlanarPosition.y,
			};
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}

void SelectOperation::applyOperation() {
	auto ball = &scene.ball;
	auto& obstacles = scene.obstacles;
	const auto& originalSelection = scene.getCurrentNode()->selectionNode->selectionState;

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