#include "Settings.h"
#include "AssetManager.h"

namespace Settings {
	std::unique_ptr<KeyBindings> Bindings;
	float UIScale = 1.0f; // TODO: load this from a config file

	void load() {
		Bindings = std::make_unique<KeyBindings>(AssetManager::loadTextFile("settings/bindings.cfg"));
	}
}