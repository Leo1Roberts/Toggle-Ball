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