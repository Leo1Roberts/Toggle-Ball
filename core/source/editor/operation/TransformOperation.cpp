#include "editor/operation/TransformOperation.h"

#include "editor/operation/RotateOperation.h"
#include "editor/operation/ScaleOperation.h"
#include "editor/operation/TranslateOperation.h"
#include "ui/UIList.h"
#include "ui/UIPanel.h"
#include "ui/UIText.h"


std::unique_ptr<TransformOperation> TransformOperation::make(ActionCode actionCode, EditorScene* scene, const Camera* camera, const TransformQuickSettings& settings, TriggerType trigger, glm::vec2 initialPlanarPosition) {
	switch (actionCode) {
	case ActionCode::Translate:
		return std::make_unique<TranslateOperation>(scene, camera, settings, trigger, initialPlanarPosition);
	case ActionCode::Rotate:
		return std::make_unique<RotateOperation>(scene, camera, settings, trigger, initialPlanarPosition);
	case ActionCode::Scale:
		return std::make_unique<ScaleOperation>(scene, camera, settings, trigger, initialPlanarPosition);
	default:;
		return nullptr;
	}
}
std::unique_ptr<TransformOperation> TransformOperation::makeFromExisting(ActionCode actionCode, const TransformOperation* existingOperation) {
	switch (actionCode) {
	case ActionCode::Translate:
		return std::make_unique<TranslateOperation>(*existingOperation);
	case ActionCode::Rotate:
		return std::make_unique<RotateOperation>(*existingOperation);
	case ActionCode::Scale:
		return std::make_unique<ScaleOperation>(*existingOperation);
	default:;
		return nullptr;
	}
}


std::vector<BindingHint> TransformOperation::getBindingHints() const {
	return {{KeyChord(KeyCode::Unknown, MOD_SHIFT), "Precision mode"}};
}

void TransformOperation::createUI(UINode& container) {
	auto panel = container.addChild(std::make_unique<UIPanel>(
		PanelStyle{
			.fillColor = {24, 26, 32, 150},
			.cornerRadius = 4.f,
		}));
	panel->setLayout({
		.anchor = Anchor::TopCentre,
		.widthMode  = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.padding = glm::vec2(6.f),
		.margin  = glm::vec2(10.f),
	});

	detailsText = panel->addChild(std::make_unique<UIText>("", TextStyle{
		.font = FontId::CourierNew,
		.fontSize = 16.f,
		.color = Color::White,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle
	}));
}

bool TransformOperation::updateUI() {
	if (!Operation::updateUI()) return false;
	return detailsText != nullptr;
}


OperationResponse TransformOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
		applyOperation();
		return {.consumedEvent = true, .status = OperationStatus::Running};
	}

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Toggle:
				case ActionCode::InstantToggle:
					if (trigger == TriggerType::ActionKey) {
						finish();
						commit();
						return {.consumedEvent = false, .status = OperationStatus::Committed};
					}
					return {.consumedEvent = true, .status = OperationStatus::Running};
				default:;
					return {.consumedEvent = false, .status = OperationStatus::Running};
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (!typing && trigger != TriggerType::ActionKey && pointer->id == 0 && (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)) {
			glm::vec2 newPointerPlanarPosition = camera->screenToPlanarPosition(pointer->position);
			updateTransformation(newPointerPlanarPosition);
			pointerPlanarPosition = newPointerPlanarPosition;
			applyOperation();
			return {.consumedEvent = false, .status = OperationStatus::Running};
		}
	} else if (auto* c = std::get_if<char>(&event)) {
		if (!typing && TextInputBuffer::Float(*c)) {
			typing = true;
			if (textInput.processEvent(*c) == TextInputEventEffect::Buffer) {
				applyOperation();
				return {.consumedEvent = true, .status = OperationStatus::Running};
			}
		}
	}
	return {.consumedEvent = false, .status = OperationStatus::Running};
}