#include "editor/EditorContext.h"

#include "ui/UIList.h"
#include "ui/UIPanel.h"
#include "ui/UIText.h"


std::unique_ptr<UIHorizontalList> EditorContext::makeShortcutHint(KeyChord keyChord, const std::string& effect) {
	auto hint = std::make_unique<UIHorizontalList>(2.f, 0.f);
	hint->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
	});

	TextStyle textStyle = {
		.fontSize = 16.f,
		.color = Color::LightGrey,
		.alignHorizontal = TextAlignHorizontal::Centre,
		.alignVertical = TextAlignVertical::Middle,
	};

	auto addKeySymbol = [&hint, textStyle](std::string_view name) {
		auto box = hint->addChild<UIPanel>(
			PanelStyle{
				.fillColor = Color::Transparent,
				.strokeColor = Color::LightGrey,
				.cornerRadius = 3.f,
				.strokeWidth = 1.f,
			});
		constexpr float paddingY = 2.f;
		if (name.length() == 1)
			box->setLayout({
				.anchor = Anchor::Centre,
				.widthMode = SizingMode::Absolute, .width = textStyle.fontSize + paddingY * 2.f,
				.heightMode = SizingMode::Absolute, .height = textStyle.fontSize + paddingY * 2.f,
			});
		else
			box->setLayout({
				.anchor = Anchor::Centre,
				.widthMode = SizingMode::Wrap,
				.heightMode = SizingMode::Wrap,
				.padding = {paddingY * 1.75f, paddingY}
			});
		auto text = box->addChild<UIText>(std::string(name), textStyle);
		text->setLayout({
			.anchor = Anchor::Centre,
			.widthMode = SizingMode::Wrap,
			.heightMode = SizingMode::Wrap,
		});
		return box;
	};

	if (keyChord.modifiers & MOD_CTRL)
		addKeySymbol(KeyRegistry::toString(KeyCode::Ctrl));
	if (keyChord.modifiers & MOD_SHIFT)
		addKeySymbol(KeyRegistry::toString(KeyCode::Shift));
	if (keyChord.modifiers & MOD_ALT)
		addKeySymbol(KeyRegistry::toString(KeyCode::Alt));
	if (keyChord.code != KeyCode::Unknown)
		addKeySymbol(KeyRegistry::toString(keyChord.code));

	auto effectNode = hint->addChild(std::make_unique<UIText>(effect, textStyle));
	effectNode->setLayout({
		.anchor = Anchor::Centre,
		.widthMode = SizingMode::Wrap,
		.heightMode = SizingMode::Wrap,
		.margin = {3.f, 0.f}
	});

	return hint;
}