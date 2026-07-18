#include "KeyBindings.h"

#include "AssetManager.h"

#include <memory>
#include <optional>
#include <sstream>


KeyBindingsContext::KeyBindingsContext(const std::string& data) {
	std::istringstream ss(data);

	std::string line;
	while (std::getline(ss, line)) {
		if (line.empty() || line[0] == '#') continue;

		size_t delimiterPosition = line.find('=');
		if (delimiterPosition == std::string::npos)
			throw std::invalid_argument("Invalid key binding: " + line);

		std::string actionString = line.substr(0, delimiterPosition);
		std::string keyString = line.substr(delimiterPosition + 1);

		auto key = KeyRegistry::fromString(keyString);
		auto action = ActionRegistry::fromString(actionString);

		if (key.has_value() && action.has_value())
			bindings[key.value()] = action.value();
		else
			throw std::invalid_argument("Unknown action/key: " + keyString + "=" += actionString);
	}
}

std::string KeyBindingsContext::serialize() const {
	std::ostringstream ss;

	for (const auto& [keyCode, actionCode] : bindings) {
		std::string_view key = KeyRegistry::toString(keyCode);
		std::string_view action = ActionRegistry::toString(actionCode);

		if (key != ActionRegistry::UNKNOWN_ACTION && action != ActionRegistry::UNKNOWN_ACTION)
			ss << action << "=" << key << std::endl;
	}

	return ss.str();
}


std::optional<ActionCode> KeyBindingsContext::translate(KeyCode key) const {
	auto bindIt = bindings.find(key);
	if (bindIt == bindings.end()) return std::nullopt;
	return bindIt->second;
}

namespace KeyBindings {
	std::unique_ptr<KeyBindingsContext> app;
	std::unique_ptr<KeyBindingsContext> game;
#if defined(PLATFORM_DESKTOP)
	std::unique_ptr<KeyBindingsContext> editor;
#endif

	void load() {
		app = std::make_unique<KeyBindingsContext>(AssetManager::loadTextFile("bindings/app.txt"));
		game = std::make_unique<KeyBindingsContext>(AssetManager::loadTextFile("bindings/game.txt"));
#if defined(PLATFORM_DESKTOP)
		editor = std::make_unique<KeyBindingsContext>(AssetManager::loadTextFile("bindings/editor.txt"));
#endif
	}
}