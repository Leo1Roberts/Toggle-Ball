#ifndef SETTINGS_H
#define SETTINGS_H

#include "KeyBindings.h"

#include <memory>


namespace Settings {
	extern std::unique_ptr<KeyBindings> Bindings;
	extern float UIScale;

	void load();
}


#endif // SETTINGS_H
