#include "Settings.h"
#include "AssetManager.h"

namespace Settings {
	std::unique_ptr<KeyBindings> bindings;

	void load() {
		bindings = std::make_unique<KeyBindings>(AssetManager::loadTextFile("settings/bindings.cfg"));
	}
}