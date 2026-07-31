#include "TranslateOperation.h"


namespace Axis {
	glm::vec2 X{1.f, 0.f};
	glm::vec2 Y{0.f, 1.f};
}


void TranslateOperation::doProcessEvent(const Event& event) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (auto actionCode = Settings::Bindings->translate(key->chord)) {
				switch (*actionCode) {
				case ActionCode::LockToXAxis:
					setMode(Axis::X);
					break;
				case ActionCode::LockToYAxis:
					setMode(Axis::Y);
					break;
				default:;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (pointer->id == 0 && pointer->action == PointerAction::Move) {
			glm::vec2 pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			rawTranslation = pointerPlanarPosition - initialPointerPlanarPosition;
			applyOperation();
		}
	}
}


void TranslateOperation::applyOperation() {
	auto ball = context->scene->getBall();
	auto& obstacles = context->scene->getObstacles();

	if (rawTranslation == glm::vec2(0.f)) return;

	auto focus = context->scene->getSelectionFocus();
	float focusAngle = 0.f;
	if (focus->type == EntityType::Obstacle)
		focusAngle = obstacles[focus->index].getKinematicState()->getAngle();

	glm::vec2 direction;
	if (constraint == ConstraintType::None)
		direction = normalize(rawTranslation);
	else if (!local && constraint == ConstraintType::GlobalAxis)
		direction = baseAxis;
	else
		direction = angleToRotation2D(focusAngle) * baseAxis;

	float magnitude = glm::dot(rawTranslation, direction);
	if (magnitude == 0.f) return; // Perpendicular

	glm::vec2 translation;
	if (!local)
		translation = direction * magnitude;

	if (ball->isSelected()) {
		if (local)
			translation = (angleToRotation2D(-focusAngle) * direction) * magnitude;
		ball->translateBy(translation, context->scene->getCurrentNode()->level.ballDescriptor.get());
	}

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			if (local)
				translation = (angleToRotation2D(obstacle.getKinematicState()->getAngle() - focusAngle) * direction) * magnitude;

			obstacle.translateBy(translation, stateless, context->scene->isToggled(),
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}
}


void TranslateOperation::setMode(glm::vec2 requestedAxis) {
	if (constraint == ConstraintType::None || (constraint == ConstraintType::GlobalAxis && baseAxis != requestedAxis)) {
		constraint = ConstraintType::GlobalAxis;
		baseAxis = requestedAxis;
	} else if (context->scene->getSelectionFocus()->type == EntityType::Obstacle &&
		((!local && constraint == ConstraintType::GlobalAxis && baseAxis == requestedAxis) || (constraint == ConstraintType::LocalAxis && baseAxis != requestedAxis))) {
		constraint = ConstraintType::LocalAxis;
		baseAxis = requestedAxis;
	} else
		constraint = ConstraintType::None;

	applyOperation();
}