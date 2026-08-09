#ifndef SETTINGS_H
#define SETTINGS_H

#include "io/KeyBindings.h"

#include <memory>


struct SizePreferences {
	float uiScale = 1.f;

	// Sizes given in pixels (before scaling)
	float outlineWidth = 3.f;
	float centreDotRadius = 3.f;
	float axisLineWidth = 1.f;
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
