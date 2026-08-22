#include "editor/operation/TranslateOperation.h"

#include "ui/UIText.h"


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
	default: return std::nullopt;
	}
}


std::vector<BindingHint> TranslateOperation::getBindingHints() const {
	std::vector<BindingHint> hints = TransformOperation::getBindingHints();
	if (trigger != TriggerType::ActionKey) {
		if (auto binding = Settings::Bindings->findBinding(ActionCode::LockToXAxis))
			hints.emplace_back(*binding, "X axis");
		if (auto binding = Settings::Bindings->findBinding(ActionCode::LockToYAxis))
			hints.emplace_back(*binding, "Y axis");
	}
	return hints;
}

bool TranslateOperation::updateUI() {
	if (!TransformOperation::updateUI()) return false;

	std::string text = "Translation ";
	if (constraint == ConstraintType::None)
		text += "X: " + floatToString(translation.x, 3, true) + "  Y: " + floatToString(translation.y, 3, true);
	else {
		if (constraint == ConstraintType::LocalAxis ||
		   (constraint == ConstraintType::GlobalAxis && settings.transformIndividually))
			text += "along local ";
		else if (constraint == ConstraintType::GlobalAxis)
			text += "along global ";

		if (baseAxis == Axis::X) text += "X: ";
		else if (baseAxis == Axis::Y) text += "Y: ";

		if (typing)
			text += textInput.getValue<const std::string&>();
		else
			text += floatToString(magnitude, 3, true);
	}

	detailsText->setText(text);

	return true;
}

void TranslateOperation::renderGizmos(GizmoRenderer& gizmoRenderer) {
	if (constraint == ConstraintType::None) return;

	col color = baseAxis == Axis::X ? Color::AxisX : Color::AxisY;
	gizmoRenderer.addLine(focusedAxisLine, {
		.primaryColor = color,
		.width = Settings::Sizes.lineWidth
	});

	color = col(color, 0.3f);
	for (auto line : otherAxisLines)
		gizmoRenderer.addLine(line, {
		.primaryColor = color,
		.width = Settings::Sizes.lineWidth
	});

	gizmoRenderer.render();
}


OperationResponse TranslateOperation::doProcessEvent(const Event& event) {
	auto response = TransformOperation::doProcessEvent(event);
	if (response.consumedEvent)
		return response;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Translate:
					return {.consumedEvent = trigger != TriggerType::ActionKey, .status = OperationStatus::Running};
				case ActionCode::LockToXAxis:
					if (trigger != TriggerType::ActionKey) {
						setConstraint(Axis::X);
						return {.consumedEvent = true, .status = OperationStatus::Running};
					} break;
				case ActionCode::LockToYAxis:
					if (trigger != TriggerType::ActionKey) {
						setConstraint(Axis::Y);
						return {.consumedEvent = true, .status = OperationStatus::Running};
					} break;
				default:;
					return {.consumedEvent = false, .status = OperationStatus::Running};
				}
			}
		}

		if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
			if (auto vector = keyToTranslationVector(key->chord.code)) {
				if (trigger == TriggerType::ActionKey) {
					rawTranslation += *vector * precisionMultiplier;
					applyOperation();
					return {.consumedEvent = true, .status = OperationStatus::Running};
				}
			}
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void TranslateOperation::applyOperation() {
	otherAxisLines.clear();

	auto ball = &scene->ball;
	auto& obstacles = scene->obstacles;

	auto focus = &scene->selectionFocus;
	float focusAngle = 0.f;
	if (focus->type == EntityType::Obstacle)
		focusAngle = obstacles[focus->index].getKinematicState()->getAngle();

	glm::vec2 direction;

	if (trigger == TriggerType::ActionKey)
		translation = rawTranslation;
	else {
		if (constraint == ConstraintType::None) {
			if (rawTranslation == glm::vec2(0.f))
				direction = glm::vec2(0.f);
			else
				direction = normalize(rawTranslation);
		} else if (!settings.transformIndividually && constraint == ConstraintType::GlobalAxis)
			direction = baseAxis;
		else
			direction = angleToRotation2D(focusAngle) * baseAxis;

		if (typing) {
			if (auto value = textInput.getValue<std::optional<float>>())
				magnitude = *value;
			else {
				updateUI();
				scene->cancelLevelChange();
				return;
			}
		} else
			magnitude = dot(rawTranslation, direction);

		if (!settings.transformIndividually)
			translation = direction * magnitude;
	}

	if (ball->isSelected()) {
		if (settings.transformIndividually && trigger != TriggerType::ActionKey)
			translation = (angleToRotation2D(-focusAngle) * direction) * magnitude;

		ball->translateBy(translation, scene->getCurrentNode()->level.ballDescriptor.get());

		if (settings.transformIndividually && constraint != ConstraintType::None && focus->type != EntityType::Ball)
			otherAxisLines.push_back({
				.point = ball->descriptor->initialPosition,
				.direction = baseAxis,
			});
	}

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			glm::vec2 individualDirection = direction;
			if (settings.transformIndividually && trigger != TriggerType::ActionKey) {
				individualDirection = angleToRotation2D(obstacle.getKinematicState()->getAngle() - focusAngle) * direction;
				translation = individualDirection * magnitude;
			}

			obstacle.translateBy(translation, settings.transformBothStates, scene->isToggled(),
				scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh();

			if (settings.transformIndividually && constraint != ConstraintType::None &&
				!(focus->type == EntityType::Obstacle && i == focus->index))
				otherAxisLines.push_back({
					.point = worldToPlanar(obstacle.getKinematicState()->getPosition()),
					.direction = individualDirection,
				});
		}
	}

	if (trigger != TriggerType::ActionKey && settings.transformIndividually)
		translation = angleToRotation2D(-focusAngle) * direction * magnitude;

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

	updateUI();
}


void TranslateOperation::setConstraint(glm::vec2 requestedAxis) {
	if (constraint == ConstraintType::None || (typing && constraint == ConstraintType::LocalAxis && baseAxis == requestedAxis) || (constraint == ConstraintType::GlobalAxis && baseAxis != requestedAxis)) {
		constraint = ConstraintType::GlobalAxis;
		baseAxis = requestedAxis;
	} else if (scene->selectionFocus.type == EntityType::Obstacle &&
		((!settings.transformIndividually && constraint == ConstraintType::GlobalAxis && baseAxis == requestedAxis) || (constraint == ConstraintType::LocalAxis && baseAxis != requestedAxis))) {
		constraint = ConstraintType::LocalAxis;
		baseAxis = requestedAxis;
	} else
		constraint = ConstraintType::None;

	applyOperation();
}