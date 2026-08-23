#include "editor/operation/RotateOperation.h"

#include "ui/UIText.h"


std::optional<float> RotateOperation::keyToRotationRadians(KeyCode key) {
	switch (key) {
	// case KeyCode::Left:  return glm::quarter_pi<float>();
	// case KeyCode::Right: return -glm::quarter_pi<float>();
	default: return std::nullopt;
	}
}


bool RotateOperation::updateUI() {
	if (!PivotOperation::updateUI()) return false;
	
	std::string text = "Rotation: ";
	text += typing ? textInput.getValue<const std::string&>() : floatToString(to_deg(rotation), 3, true);
	detailsText->setText(text);
	
	return true;
}


float RotateOperation::angleDifference(glm::vec2 newPos, glm::vec2 oldPos, glm::vec2 pivot) {
	glm::vec2 v1 = oldPos - pivot;
	glm::vec2 v2 = newPos - pivot;

	if (v1 == glm::vec2(0.f) || v2 == glm::vec2(0.f)) return 0.f;

	return std::atan2(v1.x * v2.y - v1.y * v2.x, dot(v1, v2));
}

OperationResponse RotateOperation::doProcessEvent(const Event& event) {
	auto response = PivotOperation::doProcessEvent(event);
	if (response.consumedEvent)
		return response;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				if (*actionCode == ActionCode::Rotate)
					return {.consumedEvent = trigger != TriggerType::ActionKey, .status = OperationStatus::Running};

				return {.consumedEvent = false, .status = OperationStatus::Running};
			}
		}
		if (trigger == TriggerType::ActionKey) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				if (auto amount = keyToRotationRadians(key->chord.code)) {
					if (trigger == TriggerType::ActionKey) {
						setRotation(rotation + *amount * precisionMultiplier);
						applyOperation();
						return {.consumedEvent = true, .status = OperationStatus::Running};
					}
				}
			}
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void RotateOperation::applyOperation() {
	if (typing) {
		if (auto value = textInput.getValue<std::optional<float>>())
			setRotation(to_rad(*value));
		else {
			updateUI();
			scene->cancelLevelChange();
			return;
		}
	}

	auto ball = &scene->ball;
	auto& obstacles = scene->obstacles;

	if (ball->isSelected())
		ball->rotateBy(rotationMatrix, pivot, settings.transformIndividually, scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.rotateBy(rotation, rotationMatrix, pivot, settings.transformBothStates, scene->isToggled(), settings.transformIndividually,
				scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.invalidateDomainMesh();
		}
	}

	updateUI();
}


void RotateOperation::setRotation(float radians) {
	rotation = radians;
	rotationMatrix = angleToRotation2D(rotation);
}