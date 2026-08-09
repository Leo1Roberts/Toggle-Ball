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


void TranslateOperation::renderGizmos() {
	if (constraint == ConstraintType::None) return;

	col color = baseAxis == Axis::X ? Color::AxisX : Color::AxisY;
	context->gizmoRenderer->addLine(focusedAxisLine, {
		.color = color,
		.width = Settings::Sizes.axisLineWidth
	});

	color = col(color, 0.3f);
	for (auto line : otherAxisLines)
		context->gizmoRenderer->addLine(line, {
		.color = color,
		.width = Settings::Sizes.axisLineWidth
	});

	context->gizmoRenderer->render();
}


bool TranslateOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
		applyOperation(); // Get value later - rawTranslation is not suitable
		return true;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::LockToXAxis:
					setMode(Axis::X);
					return true;
				case ActionCode::LockToYAxis:
					setMode(Axis::Y);
					return true;
				case ActionCode::Toggle:
				case ActionCode::InstantToggle:
					if (trigger == TriggerType::ActionKey) {
						finish();
						commit();
						return false;
					}
					return true;
				case ActionCode::Undo:
				case ActionCode::Redo:
				case ActionCode::ToggleTransformLocally:
				case ActionCode::ToggleTransformBothStates:
					return false;
				default:
					return true;
				}
			}
		}

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
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (trigger != TriggerType::ActionKey && pointer->id == 0 && (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)) {
			glm::vec2 pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			rawTranslation = pointerPlanarPosition - initialPointerPlanarPosition;
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c) && constraint != ConstraintType::None) {
			typing = true;
			if (textInput.processEvent(*c) == TextInputEventEffect::Buffer)
				applyOperation(); // Get value later - rawTranslation is not suitable
		}
	}
	return false;
}


void TranslateOperation::applyOperation() {
	otherAxisLines.clear();

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

		if (context->quickSettings->transformLocally && constraint != ConstraintType::None && focus->type != EntityType::Ball)
			otherAxisLines.push_back({
				.point = ball->descriptor->initialPosition,
				.direction = baseAxis,
			});
	}

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			glm::vec2 localDirection = direction;
			if (context->quickSettings->transformLocally && trigger != TriggerType::ActionKey) {
				localDirection = (angleToRotation2D(obstacle.getKinematicState()->getAngle() - focusAngle) * direction);
				translation = localDirection * magnitude;
			}

			obstacle.translateBy(translation, context->quickSettings->transformBothStates, context->scene->isToggled(),
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh(*context->uiToWorldScale);

			if (context->quickSettings->transformLocally && constraint != ConstraintType::None &&
				!(focus->type == EntityType::Obstacle && i == focus->index))
				otherAxisLines.push_back({
					.point = worldToPlanar(obstacle.getKinematicState()->getPosition()),
					.direction = localDirection,
				});
		}
	}

	glm::vec2 focusPos{};
	if (focus->type == EntityType::Ball)
		focusPos = ball->descriptor->initialPosition;
	else if (focus->type == EntityType::Obstacle)
		focusPos = worldToPlanar(obstacles[focus->index].getKinematicState()->getPosition());

	if (constraint != ConstraintType::None)
		focusedAxisLine = {
		.point = focusPos,
		.direction = direction,
	};
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