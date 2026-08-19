#include "editor/operation/ScaleOperation.h"

#include "ui/UIList.h"
#include "ui/UIText.h"


void ScaleOperation::createUI() {
	if (auto binding = Settings::Bindings->findBinding(ActionCode::Scale))
		context->operationShortcutHints->addChild(std::move(EditorContext::makeShortcutHint(*binding, "Cycle dimension")));
	if (trigger == TriggerType::TriggerKey) {
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Translate))
			context->operationShortcutHints->addChild(std::move(EditorContext::makeShortcutHint(*binding, "Translate")));
		if (auto binding = Settings::Bindings->findBinding(ActionCode::Rotate))
			context->operationShortcutHints->addChild(std::move(EditorContext::makeShortcutHint(*binding, "Rotate")));
	}

	updateDetailsText();
}

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
	detailsText->setText(text);
}


bool ScaleOperation::doProcessEvent(const Event& event) {
	if (TransformOperation::doProcessEvent(event))
		return true;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				if (*actionCode == ActionCode::Scale) {
					dimension = (Dimension)(((int)dimension + 1) % (int)Dimension::COUNT);
					applyOperation();
					return true;
				}
				return false;
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