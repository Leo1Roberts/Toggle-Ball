#include "editor/operation/TranslateOperation.h"


namespace Axis {
	glm::vec2 X{1.f, 0.f};
	glm::vec2 Y{0.f, 1.f};
} // namespace Axis


std::optional<glm::vec2> TranslateOperation::keyToTranslationVector(KeyCode key) {
	switch (key) {
	case KeyCode::Left:  return glm::vec2(-1.f, 0.f);
	case KeyCode::Right: return glm::vec2( 1.f, 0.f);
	case KeyCode::Up:    return glm::vec2( 0.f, 1.f);
	case KeyCode::Down:  return glm::vec2( 0.f,-1.f);
	default:             return std::nullopt;
	}
}


bool TranslateOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event)) {
		applyOperation(); // Get value later - rawTranslation is not suitable
		return true;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (trigger == TriggerType::ActionKey) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				if (auto vector = keyToTranslationVector(key->chord.code)) {
					rawTranslation += *vector;
					applyOperation();
					return true;
				}
			}
			return false;
		}

		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::LockToXAxis:
					setMode(Axis::X);
					return true;
				case ActionCode::LockToYAxis:
					setMode(Axis::Y);
					return true;
				default:
					return false;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (trigger != TriggerType::ActionKey && pointer->id == 0 && pointer->action == PointerAction::Move) {
			glm::vec2 pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			rawTranslation = pointerPlanarPosition - initialPointerPlanarPosition;
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c) && constraint != ConstraintType::None) {
			typing = true;
			if (textInput.processEvent(*c))
				applyOperation(); // Get value later - rawTranslation is not suitable
		}
	}
	return false;
}


void TranslateOperation::applyOperation() {
	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;

	auto focus = &context->scene->selectionFocus;
	float focusAngle = 0.f;
	if (focus->type == EntityType::Obstacle)
		focusAngle = obstacles[focus->index].getKinematicState()->getAngle();

	glm::vec2 direction;
	float magnitude = 0.f;
	glm::vec2 translation;

	if (trigger == TriggerType::ActionKey)
		translation = rawTranslation;
	else {
		if (constraint == ConstraintType::None) {
			if (rawTranslation == glm::vec2(0.f))
				direction = glm::vec2(0.f);
			else
				direction = normalize(rawTranslation);
		} else if (!context->quickSettings->transformLocally && constraint == ConstraintType::GlobalAxis)
			direction = baseAxis;
		else
			direction = angleToRotation2D(focusAngle) * baseAxis;

		if (typing)
			magnitude = textInput.getValue<float>();
		else
			magnitude = dot(rawTranslation, direction);

		if (!context->quickSettings->transformLocally)
			translation = direction * magnitude;
	}

	if (ball->isSelected()) {
		if (context->quickSettings->transformLocally && trigger != TriggerType::ActionKey)
			translation = (angleToRotation2D(-focusAngle) * direction) * magnitude;
		ball->translateBy(translation, context->scene->getCurrentNode()->level.ballDescriptor.get());
	}

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			if (context->quickSettings->transformLocally && trigger != TriggerType::ActionKey)
				translation = (angleToRotation2D(obstacle.getKinematicState()->getAngle() - focusAngle) * direction) * magnitude;

			obstacle.translateBy(translation, context->quickSettings->transformBothStates, context->scene->isToggled(),
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}
}


void TranslateOperation::setMode(glm::vec2 requestedAxis) {
	if (constraint == ConstraintType::None || (typing && constraint == ConstraintType::LocalAxis && baseAxis == requestedAxis) || (constraint == ConstraintType::GlobalAxis && baseAxis != requestedAxis)) {
		constraint = ConstraintType::GlobalAxis;
		baseAxis = requestedAxis;
	} else if (context->scene->selectionFocus.type == EntityType::Obstacle &&
		((!context->quickSettings->transformLocally && constraint == ConstraintType::GlobalAxis && baseAxis == requestedAxis) || (constraint == ConstraintType::LocalAxis && baseAxis != requestedAxis))) {
		constraint = ConstraintType::LocalAxis;
		baseAxis = requestedAxis;
	} else
		constraint = ConstraintType::None;

	applyOperation();
}