#include "KeyBindings.h"

#include <memory>
#include <optional>
#include <sstream>


KeyBindings::KeyBindings(const std::string& data) {
	std::istringstream ss(data);
	std::string line;
	
	while (std::getline(ss, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty() || line[0] == '#') continue;

		size_t delimiterPosition = line.find('=');
		if (delimiterPosition == std::string::npos)
			throw std::invalid_argument("Invalid key binding: " + line);

		std::string actionString = line.substr(0, delimiterPosition);
		std::string keyString = line.substr(delimiterPosition + 1);

		auto chord = parseChord(keyString);
		auto action = ActionRegistry::fromString(actionString);

		if (chord.has_value() && action.has_value())
			bindings[chord.value()] = action.value();
		else
			throw std::invalid_argument("Unknown action/key: " + actionString + "=" += keyString);
	}
}

std::string KeyBindings::serialize() const {
	std::ostringstream ss;

	for (const auto& [chord, actionCode] : bindings) {
		std::string keyString = formatChord(chord);
		std::string_view actionString = ActionRegistry::toString(actionCode);

		if (actionString != ActionRegistry::UNKNOWN_ACTION && chord.code != KeyCode::Unknown)
			ss << actionString << "=" << keyString << "\n";
	}

	return ss.str();
}

std::optional<KeyChord> KeyBindings::parseChord(std::string_view string) {
	KeyChord chord{KeyCode::Unknown, MOD_NONE};
	size_t start = 0;
	size_t end = string.find('+');

	while (end != std::string_view::npos) {
		std::string_view word = string.substr(start, end - start);

		if (iequals(word, "CTRL")) chord.modifiers |= MOD_CTRL;
		else if (iequals(word, "SHIFT")) chord.modifiers |= MOD_SHIFT;
		else if (iequals(word, "ALT")) chord.modifiers |= MOD_ALT;
		else return std::nullopt;

		start = end + 1;
		end = string.find('+', start);
	}

	std::string_view baseKeyString = string.substr(start);
	auto keyCode = KeyRegistry::fromString(baseKeyString);

	if (keyCode.has_value()) {
		chord.code = keyCode.value();
		return chord;
	}

	return std::nullopt;
}

std::string KeyBindings::formatChord(const KeyChord& chord) {
	std::string result;

	if (chord.modifiers & MOD_CTRL)  result += "CTRL+";
	if (chord.modifiers & MOD_SHIFT) result += "SHIFT+";
	if (chord.modifiers & MOD_ALT)   result += "ALT+";

	result += KeyRegistry::toString(chord.code);
	return result;
}


std::optional<ActionCode> KeyBindings::translate(KeyChord chord) const {
	auto it = bindings.find(chord);
	if (it != bindings.end())
		return it->second; // Exact match

	if (chord.modifiers != MOD_NONE) {
		KeyChord fallbackChord(chord.code, MOD_NONE);
		it = bindings.find(fallbackChord);
		if (it != bindings.end())
			return it->second; // Fallback - just the KeyCode without modifier keys
	}

	return std::nullopt;
}