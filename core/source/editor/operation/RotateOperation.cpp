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


std::optional<Cursor> RotateOperation::queryCursor() const {
	auto diff = Camera::planarToScreenDirection(pointerPlanarPosition - pivot);
	return Cursor{
		.style = Cursor::Style::DynamicResize,
		.dynamic = true,
		.angle = std::atan2(diff.y, diff.x),
	};
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
			ctx.scene.cancelLevelChange();
			return;
		}
	}

	auto ball = &ctx.scene.ball;
	auto& obstacles = ctx.scene.obstacles;

	if (ball->isSelected())
		ball->rotateBy(rotationMatrix, pivot, ctx.quickSettings.transform.individually, ctx.scene.getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.rotateBy(rotation, rotationMatrix, ctx.quickSettings.transform.individually ? glm::vec2(0.f) : pivot, ctx.quickSettings.transform.bothStates, ctx.scene.isToggled(), ctx.quickSettings.transform.individually,
				ctx.scene.getCurrentNode()->level.obstacleDescriptors[i].get());

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