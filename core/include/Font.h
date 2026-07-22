#ifndef FONT_H
#define FONT_H

#include "main.h"


#include <memory>


struct CharBounds {
	float left; float top; float right; float bottom;
};

class Texture;

struct Typeface {
	explicit Typeface(const std::string& name);

	std::unique_ptr<Texture> texture;
	CharBounds charLocations[256]{};
	float size;
	float digitWidth = 0;
	float maxCharWidth = 0;
};

struct Font {
	const Typeface* typeface{};
	bool monoSpaced = false;
	float charSpacing = 0.f;
	float wordSpacing = 0.f;
};

enum class FontId : byte {
	Bahnschrift,
	CourierNew,
};

namespace Fonts {
	namespace Typefaces {
		extern std::unique_ptr<Typeface> Bahnschrift;
		extern std::unique_ptr<Typeface> CourierNew;

		void load();
	}

	extern std::unique_ptr<Font> Bahnschrift;
	extern std::unique_ptr<Font> CourierNew;

	void load();
	const Font* get(FontId id);
}


#endif // FONT_H
