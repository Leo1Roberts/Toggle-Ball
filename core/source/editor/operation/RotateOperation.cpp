#include "editor/operation/RotateOperation.h"

#include "ui/UIList.h"
#include "ui/UIText.h"


void RotateOperation::createUI() {
	if (trigger == TriggerType::TriggerKey) {
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Translate))
			context->operationShortcutHints->addChild(std::move(EditorContext::makeShortcutHint(*binding, "Translate")));
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Scale))
			context->operationShortcutHints->addChild(std::move(EditorContext::makeShortcutHint(*binding, "Scale")));
	}

	updateDetailsText();
}

void RotateOperation::updateDetailsText() {
	std::string text = "Rotation: ";
	text += typing ? textInput.getValue<const std::string&>() : floatToString(to_deg(rotation), 3, true);
	detailsText->setText(text);
}


std::optional<float> RotateOperation::keyToRotationRadians(KeyCode key) {
	switch (key) {
	// case KeyCode::Left:  return glm::quarter_pi<float>();
	// case KeyCode::Right: return -glm::quarter_pi<float>();
	default: return std::nullopt;
	}
}


float angleDifference(glm::vec2 newPos, glm::vec2 oldPos, glm::vec2 pivot) {
	glm::vec2 v1 = oldPos - pivot;
	glm::vec2 v2 = newPos - pivot;

	if (v1 == glm::vec2(0.f) || v2 == glm::vec2(0.f)) return 0.f;

	return std::atan2(v1.x * v2.y - v1.y * v2.x, dot(v1, v2));
}

bool RotateOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
		applyOperation();
		return true;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Translate:
				case ActionCode::Scale:
					if (trigger != TriggerType::TriggerKey) return true;
				case ActionCode::Undo:
				case ActionCode::Redo:
				case ActionCode::ToggleTransformIndividually:
				case ActionCode::ToggleTransformBothStates:
					return false;
				default:
					return true;
				}
			}
		}

		if (trigger == TriggerType::ActionKey) {
			if (key->action == KeyAction::Down || key->action == KeyAction::Repeat) {
				if (auto amount = keyToRotationRadians(key->chord.code)) {
					setRotation(rotation + *amount);
					applyOperation();
					return true;
				}
			}
			return false;
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (!typing && trigger != TriggerType::ActionKey && pointer->id == 0 && (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)) {
			glm::vec2 newPointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			setRotation(rotation + angleDifference(newPointerPlanarPosition, pointerPlanarPosition, pivot));
			pointerPlanarPosition = newPointerPlanarPosition;
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c)) {
			typing = true;
			if (textInput.processEvent(*c) == TextInputEventEffect::Buffer) {
				applyOperation();
				return true;
			}
		}
	}
	return false;
}


void RotateOperation::applyOperation() {
	if (typing) {
		if (auto value = textInput.getValue<std::optional<float>>())
			setRotation(to_rad(*value));
		else {
			updateDetailsText();
			context->scene->cancelLevelChange();
			return;
		}
	}

	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;

	if (ball->isSelected())
		ball->rotateBy(rotationMatrix, pivot, context->quickSettings->transformIndividually, context->scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.rotateBy(rotation, rotationMatrix, pivot, context->quickSettings->transformBothStates, context->scene->isToggled(), context->quickSettings->transformIndividually,
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}

	updateDetailsText();
}


void RotateOperation::setRotation(float radians) {
	rotation = radians;
	rotationMatrix = angleToRotation2D(rotation);
}