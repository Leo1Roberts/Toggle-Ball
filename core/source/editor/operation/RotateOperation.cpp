#include "editor/operation/RotateOperation.h"


RotateOperation::RotateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition)
	: TransformOperation(context, trigger, initialPointerPosition) {
	lastPointerPlanarPosition = initialPointerPlanarPosition;

	auto focus = context->scene->selectionFocus;
	if (focus.type == EntityType::Ball)
		pivot = context->scene->ball.descriptor->initialPosition;
	else if (focus.type == EntityType::Obstacle)
		pivot = worldToPlanar(context->scene->obstacles[focus.index].getKinematicState()->getPosition());
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
		setRotation(to_rad(textInput.getValue<float>()));
		applyOperation();
		return true;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
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
			glm::vec2 pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			setRotation(rotation + angleDifference(pointerPlanarPosition, lastPointerPlanarPosition, pivot));
			lastPointerPlanarPosition = pointerPlanarPosition;
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c)) {
			typing = true;
			if (textInput.processEvent(*c) == TextInputEventEffect::Buffer) {
				setRotation(to_rad(textInput.getValue<float>()));
				applyOperation();
			}
		}
	}
	return false;
}


void RotateOperation::applyOperation() {
	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;

	if (ball->isSelected())
		ball->rotateBy(rotationMatrix, pivot, context->scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.rotateBy(rotation, rotationMatrix, pivot, context->quickSettings->transformBothStates, context->scene->isToggled(), context->quickSettings->transformLocally,
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}
}


void RotateOperation::setRotation(float radians) {
	rotation = radians;
	rotationMatrix = angleToRotation2D(rotation);
}