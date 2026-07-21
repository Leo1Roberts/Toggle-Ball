#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

#include "Utilities.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>


enum ModifierFlags : byte {
	MOD_NONE	= 0,
	MOD_CTRL	= 1 << 0,
	MOD_SHIFT	= 1 << 1,
	MOD_ALT		= 1 << 2
};

enum class KeyCode {
	Enter,
	Escape,
	Space,
	Z,
	F11,
	Unknown
};

struct KeyChord {
	KeyCode code;
	byte modifiers;

	bool operator==(const KeyChord& other) const {
		return code == other.code && modifiers == other.modifiers;
	}
};

template<>
struct std::hash<KeyChord> {
	size_t operator()(const KeyChord& chord) const noexcept {
		return (static_cast<size_t>(chord.code) << 8) | chord.modifiers;
	}
};


enum class ActionCode {
	Confirm,
	Cancel,

	Quit,
	Fullscreen,

	Toggle,

	Undo,
	Redo,
};


class ActionRegistry {
public:
	constexpr static std::string_view UNKNOWN_ACTION = "UnknownAction";

	static std::optional<ActionCode> fromString(std::string_view str) {
		auto it = std::ranges::find_if(entries,
			[str](const Entry& e) { return iequals(e.name, str); });
		return (it != std::ranges::end(entries)) ? std::make_optional(it->code) : std::nullopt;
	}

	static std::string_view toString(ActionCode code) {
		auto it = std::ranges::find_if(entries,
			[code](const Entry& e) { return e.code == code; });
		return (it != std::ranges::end(entries)) ? it->name : UNKNOWN_ACTION;
	}

private:
	struct Entry { ActionCode code; std::string_view name; };

	static constexpr Entry entries[] = {
		{ ActionCode::Confirm,		"Confirm" },
		{ ActionCode::Cancel,		"Cancel" },
		{ ActionCode::Quit,			"Quit" },
		{ ActionCode::Fullscreen,	"Fullscreen" },
		{ ActionCode::Toggle,		"Toggle" },
		{ ActionCode::Undo,			"Undo" },
		{ ActionCode::Redo,			"Redo" },
	};
};

class KeyRegistry {
public:
	constexpr static std::string_view UNKNOWN_KEY = "UnknownKey";

	static std::optional<KeyCode> fromString(std::string_view str) {
		auto it = std::ranges::find_if(entries,
			[str](const Entry& e) { return iequals(e.name, str); });
		return (it != std::ranges::end(entries)) ? std::make_optional(it->code) : std::nullopt;
	}

	static std::string_view toString(KeyCode code) {
		auto it = std::ranges::find_if(entries,
			[code](const Entry& e) { return e.code == code; });
		return (it != std::ranges::end(entries)) ? it->name : UNKNOWN_KEY;
	}

private:
	struct Entry { KeyCode code; std::string_view name; };

	static constexpr Entry entries[] = {
		{ KeyCode::Enter,	"ENTER" },
		{ KeyCode::Escape,	"ESC" },
		{ KeyCode::Space,	"SPACE" },
		{ KeyCode::Z,		"Z" },
		{ KeyCode::F11,		"F11" },
	};
};


class KeyBindings {
public:
	KeyBindings(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	void bind(KeyChord chord, ActionCode action) {
		bindings[chord] = action;
	}

	[[nodiscard]] std::optional<ActionCode> translate(KeyChord chord) const;

private:
	[[nodiscard]] static std::optional<KeyChord> parseChord(std::string_view string);
	[[nodiscard]] static std::string formatChord(const KeyChord& chord);

	std::unordered_map<KeyChord, ActionCode> bindings;
};


#endif // KEY_BINDINGS_H