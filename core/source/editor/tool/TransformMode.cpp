#include "editor/tool/TransformMode.h"

#include "editor/EditorContext.h"
#include "editor/operation/RotateOperation.h"
#include "editor/operation/SelectOperation.h"
#include "editor/operation/TranslateOperation.h"


void TransformMode::addGizmos(GizmoRenderer& gizmoRenderer) const {
	if (activeOperation)
		activeOperation->addGizmos(gizmoRenderer);

	auto addCentreDot = [&gizmoRenderer, this](const EditorObstacle& obstacle, bool focus = false) {
		float opacity = 1.f; // Duplicated in EditorScreen::getObstacleOpacity
		auto motionSpec = obstacle.descriptor->motion.get();
		if (dynamic_cast<OscillatingPositionSpec*>(motionSpec) ||
			dynamic_cast<OscillatingAngleSpec*>(motionSpec))
			// Includes a small period where the obstacle is completely invisible
			opacity = std::clamp(2.5f * std::abs(0.5f - ctx.scene.getTogglePosition()) - 0.2f, 0.f, 1.f);

		col color = focus ? Color::Focused : Color::Selected;
		color.a = (byte)((float)color.a * opacity);
		PanelStyle style = {
			.fillColor = color,
			.cornerRadius = Settings::Sizes.centreDotRadius,
		};
		gizmoRenderer.addCircle(worldToPlanar(obstacle.getKinematicState()->getPosition()), style);
	};
	for (int i = 0; i < ctx.scene.obstacles.size(); i++) {
		const auto& obstacle = ctx.scene.obstacles[i];
		if (obstacle.isSelected() &&
			!(ctx.scene.selectionFocus.type == EntityType::Obstacle && ctx.scene.selectionFocus.index == i))
			addCentreDot(obstacle);
	}
	if (ctx.scene.selectionFocus.type == EntityType::Obstacle)
		addCentreDot(ctx.scene.obstacles[ctx.scene.selectionFocus.index], true);
}


std::unique_ptr<Operation> TransformMode::startDrag(const PointerEvent& dragStartEvent) {
	auto pointerPlanarPosition = ctx.camera.screenToPlanarPosition(pointerDownEvent.position);
	bool hit = pointedAtEntity(pointerPlanarPosition);

	if (hit) {
		auto selectOperation = SelectOperation(ctx, TriggerType::Pointer, pointerPlanarPosition, true);
		if (selectOperation.start(pointerDownEvent.modifiers))
			selectOperation.finish();
		else return nullptr;

		if (selectOperation.getMode() == SelectionMode::Subtract) {
			selectOperation.cancel();
			return nullptr;
		}
	}

	if (dragStartEvent.button == PointerButton::Primary) {
		if (hit) {
			auto translateOperation = std::make_unique<TranslateOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition);
			if (translateOperation->start(pointerDownEvent.modifiers))
				return translateOperation;
			return nullptr;
		}

		auto selectOperation = std::make_unique<SelectOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition);
		if (selectOperation->start(pointerDownEvent.modifiers))
			return selectOperation;
		return nullptr;
	}

	if (dragStartEvent.button == PointerButton::Secondary) {
		if (hit) {
			auto rotateOperation = std::make_unique<RotateOperation>(ctx, TriggerType::Pointer, pointerPlanarPosition);
			if (rotateOperation->start(pointerDownEvent.modifiers))
				return rotateOperation;
			return nullptr;
		}
	}

	return nullptr;
}