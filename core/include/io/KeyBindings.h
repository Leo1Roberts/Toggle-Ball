#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

#include "utilities/Utilities.h"

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
	Ctrl, Shift, Alt,
	Tab,
	Enter, Escape,
	Space, Backspace, Delete,

	A, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

	Num0, Num1, Num2, Num3, Num4,
	Num5, Num6, Num7, Num8, Num9,

	Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
	Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,

	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
	F11, F12, F13, F14, F15, F16, F17, F18, F19, F20,
	F21, F22, F23, F24, F25,

	Up, Down, Left, Right,
	Home, End, PageUp, PageDown,

	Unknown
};

struct KeyChord {
	KeyCode code;
	byte modifiers;

	bool operator==(const KeyChord&) const = default;
};

template<>
struct std::hash<KeyChord> {
	size_t operator()(const KeyChord& chord) const noexcept {
		return ((size_t)chord.code << 8) | chord.modifiers;
	}
};


enum class ActionCode {
	Quit,
	Fullscreen,

	Copy, Paste,

	TestLevel,

	Toggle, InstantToggle,

	Undo, Redo,

	SelectAll, DeselectAll,

	Translate,
	Rotate,
	Scale,

	LockToXAxis, LockToYAxis,

	ToggleTransformBothStates, ToggleTransformIndividually,
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
		{ ActionCode::Quit,                        "Quit"                          },
		{ ActionCode::Fullscreen,                  "Fullscreen"                    },
		{ ActionCode::Copy,                        "Copy"                          },
		{ ActionCode::Paste,                       "Paste"                         },
		{ ActionCode::TestLevel,                   "Test level"                    },
		{ ActionCode::Toggle,                      "Toggle"                        },
		{ ActionCode::InstantToggle,               "Instant toggle"                },
		{ ActionCode::Undo,                        "Undo"                          },
		{ ActionCode::Redo,                        "Redo"                          },
		{ ActionCode::SelectAll,                   "Select all"                    },
		{ ActionCode::DeselectAll,                 "Deselect all"                  },
		{ ActionCode::Translate,                   "Translate"                     },
		{ ActionCode::Rotate,                      "Rotate"                        },
		{ ActionCode::Scale,                       "Scale"                         },
		{ ActionCode::LockToXAxis,                 "Constrain to X axis"           },
		{ ActionCode::LockToYAxis,                 "Constrain to Y axis"           },
		{ ActionCode::ToggleTransformBothStates,   "Toggle transform both states"  },
		{ ActionCode::ToggleTransformIndividually, "Toggle transform individually" },
	};
};

class KeyRegistry {
public:
	constexpr static std::string UNKNOWN_KEY = "UnknownKey";

	static std::optional<KeyCode> fromString(std::string_view str) {
		auto it = std::ranges::find_if(entries,
			[str](const Entry& e) { return iequals(e.name, str); });
		return (it != std::ranges::end(entries)) ? std::make_optional(it->code) : std::nullopt;
	}

	static std::string toString(KeyCode code) {
		auto it = std::ranges::find_if(entries,
			[code](const Entry& e) { return e.code == code; });
		return (it != std::ranges::end(entries)) ? std::string(it->name) : UNKNOWN_KEY;
	}

private:
	struct Entry { KeyCode code; std::string_view name; };

	static constexpr Entry entries[] = {
		{ KeyCode::Ctrl , "CTRL"  },
		{ KeyCode::Shift, "SHIFT" },
		{ KeyCode::Alt,   "ALT"   },

	    { KeyCode::Tab,       "TAB"       },
	    { KeyCode::Enter,     "ENTER"     },
	    { KeyCode::Escape,    "ESCAPE"    },
	    { KeyCode::Space,     "SPACE"     },
	    { KeyCode::Backspace, "BACKSPACE" },
	    { KeyCode::Delete,    "DELETE"    },

	    { KeyCode::A, "A" }, { KeyCode::B, "B" }, { KeyCode::C, "C" },
	    { KeyCode::D, "D" }, { KeyCode::E, "E" }, { KeyCode::F, "F" },
	    { KeyCode::G, "G" }, { KeyCode::H, "H" }, { KeyCode::I, "I" },
	    { KeyCode::J, "J" }, { KeyCode::K, "K" }, { KeyCode::L, "L" },
	    { KeyCode::M, "M" }, { KeyCode::N, "N" }, { KeyCode::O, "O" },
	    { KeyCode::P, "P" }, { KeyCode::Q, "Q" }, { KeyCode::R, "R" },
	    { KeyCode::S, "S" }, { KeyCode::T, "T" }, { KeyCode::U, "U" },
	    { KeyCode::V, "V" }, { KeyCode::W, "W" }, { KeyCode::X, "X" },
	    { KeyCode::Y, "Y" }, { KeyCode::Z, "Z" },

	    { KeyCode::Num0, "0" }, { KeyCode::Num1, "1" }, { KeyCode::Num2, "2" },
	    { KeyCode::Num3, "3" }, { KeyCode::Num4, "4" }, { KeyCode::Num5, "5" },
	    { KeyCode::Num6, "6" }, { KeyCode::Num7, "7" }, { KeyCode::Num8, "8" },
	    { KeyCode::Num9, "9" },

	    { KeyCode::Numpad0, "NUMPAD0" }, { KeyCode::Numpad1, "NUMPAD1" },
	    { KeyCode::Numpad2, "NUMPAD2" }, { KeyCode::Numpad3, "NUMPAD3" },
	    { KeyCode::Numpad4, "NUMPAD4" }, { KeyCode::Numpad5, "NUMPAD5" },
	    { KeyCode::Numpad6, "NUMPAD6" }, { KeyCode::Numpad7, "NUMPAD7" },
	    { KeyCode::Numpad8, "NUMPAD8" }, { KeyCode::Numpad9, "NUMPAD9" },

	    { KeyCode::F1,  "F1"  }, { KeyCode::F2,  "F2"  }, { KeyCode::F3,  "F3"  },
	    { KeyCode::F4,  "F4"  }, { KeyCode::F5,  "F5"  }, { KeyCode::F6,  "F6"  },
	    { KeyCode::F7,  "F7"  }, { KeyCode::F8,  "F8"  }, { KeyCode::F9,  "F9"  },
	    { KeyCode::F10, "F10" }, { KeyCode::F11, "F11" }, { KeyCode::F12, "F12" },
	    { KeyCode::F13, "F13" }, { KeyCode::F14, "F14" }, { KeyCode::F15, "F15" },
	    { KeyCode::F16, "F16" }, { KeyCode::F17, "F17" }, { KeyCode::F18, "F18" },
	    { KeyCode::F19, "F19" }, { KeyCode::F20, "F20" }, { KeyCode::F21, "F21" },
	    { KeyCode::F22, "F22" }, { KeyCode::F23, "F23" }, { KeyCode::F24, "F24" },
	    { KeyCode::F25, "F25" },

	    { KeyCode::Up,       "UP"        },
	    { KeyCode::Down,     "DOWN"      },
	    { KeyCode::Left,     "LEFT"      },
	    { KeyCode::Right,    "RIGHT"     },
	    { KeyCode::Home,     "HOME"      },
	    { KeyCode::End,      "END"       },
	    { KeyCode::PageUp,   "PAGE UP"   },
	    { KeyCode::PageDown, "PAGE DOWN" },
	};
};


class KeyBindings {
public:
	KeyBindings(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	void bind(KeyChord chord, ActionCode action) {
		bindings[chord] = action;
		reverseBindings[action] = chord;
	}

	[[nodiscard]] std::optional<ActionCode> translate(KeyChord chord) const;
	[[nodiscard]] std::optional<KeyChord> findBinding(ActionCode action) const;

private:
	[[nodiscard]] static std::optional<KeyChord> parseChord(std::string_view string);
	[[nodiscard]] static std::string formatChord(const KeyChord& chord);

	std::unordered_map<KeyChord, ActionCode> bindings;
	std::unordered_map<ActionCode, KeyChord> reverseBindings;
};


struct BindingHint {
	KeyChord keyChord;
	std::string label;
};


#endif // KEY_BINDINGS_H