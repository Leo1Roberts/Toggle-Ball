#include "editor/operation/TransformOperation.h"

#include "ui/UIList.h"
#include "ui/UIPanel.h"
#include "ui/UIText.h"


void TransformOperation::createUI() {
	auto panel = context->operationUI->addChild(std::make_unique<UIPanel>(
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

	context->operationShortcutHints->addChild(EditorContext::makeShortcutHint(
		KeyChord(KeyCode::Unknown, MOD_SHIFT), "Precision mode"));
}


bool TransformOperation::doProcessEvent(const Event& event) {
	if (typing && textInput.processEvent(event) == TextInputEventEffect::Buffer) {
		applyOperation();
		return true;
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
						return false;
					}
					return true;
				default:
					return false;
				}
			}
		}
	} else if (auto* pointer = std::get_if<PointerEvent>(&event)) {
		if (!typing && trigger != TriggerType::ActionKey && pointer->id == 0 && (pointer->action == PointerAction::Move || pointer->action == PointerAction::Drag)) {
			glm::vec2 newPointerPlanarPosition = context->camera->screenToPlanarPosition(pointer->position);
			updateTransformation(newPointerPlanarPosition);
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