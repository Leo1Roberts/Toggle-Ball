#include "editor/operation/ScaleOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


bool ScaleOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
		scale = textInput.getValue<float>();
		applyOperation();
		return true;
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Scale:
					dimension = (Dimension)(((int)dimension + 1) % (int)Dimension::COUNT);
					applyOperation();
					return true;
				case ActionCode::Undo:
				case ActionCode::Redo:
				case ActionCode::ToggleTransformLocally:
				case ActionCode::ToggleTransformBothStates:
				case ActionCode::Translate:
				case ActionCode::Rotate:
					return false;
				default:
					return true;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (!typing && trigger != TriggerType::ActionKey && pointer->id == 0 && (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)) {
			pointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			scale = std::sqrt(length2(pointerPlanarPosition - pivot) / length2(initialPointerPlanarPosition - pivot));
			applyOperation();
			return false; // Allow pointer move events to pass through
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c)) {
			typing = true;
			if (textInput.processEvent(*c) == TextInputEventEffect::Buffer) {
				scale = textInput.getValue<float>();
				applyOperation();
			}
		}
	}
	return false;
}


void ScaleOperation::applyOperation() {
	auto ball = &context->scene->ball;
	auto& obstacles = context->scene->obstacles;

	bool affectMinorRadius = false;
	bool affectMajorRadius = false;
	switch (dimension) {
	case Dimension::MajorAndMinor:
		affectMinorRadius = true;
		affectMajorRadius = true;
		break;
	case Dimension::Major:
		affectMajorRadius = true;
		break;
	case Dimension::Minor:
		affectMinorRadius = true;
		break;
	default:;
	}

	if (ball->isSelected())
		ball->scaleBy(scale, pivot, context->quickSettings->transformLocally,
			context->scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.scaleBy(scale, pivot, context->quickSettings->transformLocally,
				affectMinorRadius, affectMajorRadius,
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			if (affectMinorRadius || affectMajorRadius)
				obstacle.generateMeshes(*context->uiToWorldScale);
			else
				obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}
}