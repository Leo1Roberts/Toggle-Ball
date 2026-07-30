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

	float magnitude;
	glm::vec2 direction;
	if (lockToAxis) {
		glm::vec2 readAxis;
		if (localRead) {
			auto focus = context->scene->getSelectionFocus();
			if (focus->type == EntityType::Ball)
				readAxis = baseAxis;
			else if (focus->type == EntityType::Obstacle) {
				float angle = obstacles[focus->index].getKinematicState()->getAngle();
				readAxis = angleToRotation2D(angle) * baseAxis;
			}
			direction = local ? baseAxis : readAxis;
		} else
			direction = readAxis = baseAxis;

		magnitude = glm::dot(rawTranslation, readAxis);
	} else {
		magnitude = glm::length(rawTranslation);
		if (magnitude > 0.f)
			direction = rawTranslation / magnitude;
	}

	if (magnitude == 0.f)
		return;

	glm::vec2 translation;
	if (!local)
		translation = direction * magnitude;

	if (ball->isSelected())
		ball->translateBy(translation, context->scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			if (local)
				translation = (angleToRotation2D(obstacle.getKinematicState()->getAngle()) * direction) * magnitude;

			obstacle.translateBy(translation, stateless, context->scene->isToggled(),
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}
}


void TranslateOperation::setMode(glm::vec2 requestedAxis) {
	if (!lockToAxis || (!localRead && baseAxis != requestedAxis)) {
		lockToAxis = true;
		baseAxis = requestedAxis;
	} else if (!localRead || baseAxis != requestedAxis) {
		localRead = true;
		baseAxis = requestedAxis;
	} else
		localRead = lockToAxis = false;
}