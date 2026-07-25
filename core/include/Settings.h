#ifndef SETTINGS_H
#define SETTINGS_H

#include "KeyBindings.h"

#include <memory>


struct SizePreferences {
	float uiScale = 1.f;

	// Sizes given in pixels (before scaling)
	float outlineWidth = 4;
};


struct ColorPreferences {
	float domainOpacity = 0.35f;
};


namespace Settings {
	extern std::unique_ptr<KeyBindings> Bindings;
	extern SizePreferences Sizes;
	extern ColorPreferences Colors;

	void load();
}


#endif // SETTINGS_H
