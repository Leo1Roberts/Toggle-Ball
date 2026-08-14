#include "editor/operation/TransformOperation.h"

#include "ui/UITextBubble.h"


void TransformOperation::createUI() {
	detailsBubble = context->operationUI->addChild(std::make_unique<UITextBubble>("", glm::vec2(6.f),
		PanelStyle{
			.fillColor = {24, 26, 32, 150},
			.cornerRadius = 4.f,
		},
		TextStyle{
			.font = FontId::CourierNew,
			.fontSize = 16.f,
			.color = Color::White,
			.alignHorizontal = TextAlignHorizontal::Centre,
			.alignVertical = TextAlignVertical::Middle
		}));
	detailsBubble->layout = {
		.anchor = Anchor::TopCentre,
		.offset = {0.f, 10.f}
	};
}