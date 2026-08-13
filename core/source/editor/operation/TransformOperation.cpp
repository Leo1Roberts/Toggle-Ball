#include "editor/operation/TransformOperation.h"

#include "ui/UIPanel.h"
#include "ui/UIText.h"


void TransformOperation::createUI() {
	auto detailsPopup = context->operationUI->addChild(std::make_unique<UIPanel>(PanelStyle{
		.fillColor = {24, 26, 32, 150},
		.cornerRadius = 4.f,
	}));
	detailsPopup->layout = {
		.anchor = Anchor::TopCentre,
		.widthMode = SizingMode::Absolute, .heightMode = SizingMode::Absolute,
		.width = 350.f, .height = 30.f,
		.offset = {0.f, 10.f}
	};

	detailsText = detailsPopup->addChild(std::make_unique<UIText>("", TextStyle{
		.font = FontId::CourierNew,
		.fontSize = 16.f,
		.color = Color::White,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle
	}));
}