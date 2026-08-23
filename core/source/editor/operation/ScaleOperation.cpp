#include "editor/operation/ScaleOperation.h"

#include "ui/UIText.h"


std::vector<BindingHint> ScaleOperation::getBindingHints() const {
	std::vector<BindingHint> hints = TransformOperation::getBindingHints();
	if (auto binding = Settings::Bindings->findBinding(ActionCode::Scale))
		hints.emplace_back(*binding, "Cycle dimension");
	return hints;
}

bool ScaleOperation::updateUI() {
	if (!PivotOperation::updateUI()) return false;

	std::string dimensionString;
	switch (dimension) {
	case Dimension::Major:
		dimensionString = " length";
		break;
	case Dimension::Minor:
		dimensionString = " width";
		break;
	default:;
		dimensionString = "";
	}
	std::string text = "Scale" + dimensionString + ": ";
	text += typing ? textInput.getValue<const std::string&>() : floatToString(scale, 3, true);
	detailsText->setText(text);

	return true;
}


OperationResponse ScaleOperation::doProcessEvent(const Event& event) {
	auto response = PivotOperation::doProcessEvent(event);
	if (response.consumedEvent)
		return response;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				if (*actionCode == ActionCode::Scale) {
					dimension = (Dimension)(((int)dimension + 1) % (int)Dimension::COUNT);
					applyOperation();
					return {.consumedEvent = true, .status = OperationStatus::Running};
				}
			}
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}


void ScaleOperation::applyOperation() {
	if (typing) {
		if (auto value = textInput.getValue<std::optional<float>>())
			scale = *value;
		else {
			updateUI();
			scene->cancelLevelChange();
			return;
		}
	}

	auto ball = &scene->ball;
	auto& obstacles = scene->obstacles;

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
		ball->scaleBy(scale, pivot, settings.transformIndividually,
			scene->getCurrentNode()->level.ballDescriptor.get());

	for (int i = 0; i < obstacles.size(); i++) {
		auto& obstacle = obstacles[i];
		if (obstacle.isSelected()) {
			obstacle.scaleBy(scale, pivot, settings.transformIndividually,
				affectMinorRadius, affectMajorRadius,
				scene->getCurrentNode()->level.obstacleDescriptors[i].get());

			obstacle.initKinematicState();
			if (affectMinorRadius || affectMajorRadius)
				obstacle.invalidateAllMeshes();
			else
				obstacle.invalidateDomainMesh();
		}
	}

	updateUI();
}