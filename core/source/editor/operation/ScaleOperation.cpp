#include "editor/operation/ScaleOperation.h"

#include "ui/UiTextBubble.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


void ScaleOperation::updateDetailsText() {
	std::string dimensionString;
	switch (dimension) {
	case Dimension::Major:
		dimensionString = " length";
		break;
	case Dimension::Minor:
		dimensionString = " width";
		break;
	default:
		dimensionString = "";
	}
	std::string text = "Scale" + dimensionString + ": ";
	text += typing ? textInput.getValue<const std::string&>() : floatToString(scale, 3, true);
	detailsBubble->setText(text);
	detailsBubble->updateBounds(detailsBubble->getParent()->getAbsoluteBounds());
}


bool ScaleOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
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
				case ActionCode::ToggleTransformIndividually:
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
				applyOperation();
				return true;
			}
		}
	}
	return false;
}


void ScaleOperation::applyOperation() {
	if (typing) {
		if (auto value = textInput.getValue<std::optional<float>>())
			scale = *value;
		else {
			updateDetailsText();
			context->scene->cancelLevelChange();
			return;
		}
	}

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
		ball->scaleBy(scale, pivot, context->quickSettings->transformIndividually,
			context->scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.scaleBy(scale, pivot, context->quickSettings->transformIndividually,
				affectMinorRadius, affectMajorRadius,
				context->scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			if (affectMinorRadius || affectMajorRadius)
				obstacle.generateMeshes(*context->uiToWorldScale);
			else
				obstacle.generateDomainMesh(*context->uiToWorldScale);
		}
	}

	updateDetailsText();
}