#include "Font.h"

#include "AssetManager.h"
#include "Texture.h"


Typeface::Typeface(const std::string& name) {
	struct charInfo {
		char c;
		char _spacer1_ [[maybe_unused]], _spacer2_ [[maybe_unused]], _spacer3_ [[maybe_unused]];
		CharBounds bounds;
	};

	std::string path = "fonts/" + name;
	std::string texturePath = path + ".png";
	std::string infoPath = path + ".bin";

	texture = std::make_unique<Texture>(texturePath, true);

	std::vector<byte> buffer = AssetManager::loadAssetToBuffer(infoPath);
	const auto* info = reinterpret_cast<const charInfo*>(buffer.data());
	size_t length = buffer.size() / sizeof(charInfo);

	for (int i = 0; i < length; i++) {
		charLocations[info[i].c] = info[i].bounds;

		float charWidth = info[i].bounds.right - info[i].bounds.left;
		if (info[i].c >= '0' && info[i].c <= '9')
			digitWidth = std::max(digitWidth, charWidth);
		maxCharWidth = std::max(maxCharWidth, charWidth);
	}

	size = info[0].bounds.bottom - info[0].bounds.top;
}


namespace Fonts {
	namespace Typefaces {
		std::unique_ptr<Typeface> Bahnschrift;
		std::unique_ptr<Typeface> CourierNew;

		void load() {
			Bahnschrift = std::make_unique<Typeface>("Bahnschrift");
			CourierNew = std::make_unique<Typeface>("Courier New");
		}
	}

	std::unique_ptr<Font> Bahnschrift;
	std::unique_ptr<Font> CourierNew;

	void load() {
		Typefaces::load();
		Bahnschrift = std::make_unique<Font>(Typefaces::Bahnschrift.get(), false, 0.008f, 0.02f);
		CourierNew = std::make_unique<Font>(Typefaces::CourierNew.get(), true, -0.004f, 0.06f);
	}

	const Font* get(FontId id) {
		switch (id) {
		case FontId::Bahnschrift:	return Bahnschrift.get();
	    case FontId::CourierNew:	return CourierNew.get();
		default:					return nullptr;
		}
	}
}