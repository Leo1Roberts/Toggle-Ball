#ifndef KEY_BINDINGS_H
#define KEY_BINDINGS_H

#include <optional>
#include <string>
#include <unordered_map>
#include <memory>


enum class KeyCode {
	Space,
	Unknown
};
enum class ActionCode {
	Toggle
};


class ActionRegistry {
public:
	constexpr static std::string_view UNKNOWN_ACTION = "UnknownAction";

	static std::optional<ActionCode> fromString(std::string_view str) {
		auto it = std::ranges::find_if(entries,
			[str](const Entry& e) { return e.name == str; });
		return (it != std::end(entries)) ? std::make_optional(it->code) : std::nullopt;
	}

	static std::string_view toString(ActionCode code) {
		auto it = std::ranges::find_if(entries,
			[code](const Entry& e) { return e.code == code; });
		return (it != std::end(entries)) ? it->name : UNKNOWN_ACTION;
	}

private:
	struct Entry { ActionCode code; std::string_view name; };

	static constexpr Entry entries[] = {
		{ ActionCode::Toggle, "Toggle" }
	};
};

class KeyRegistry {
public:
	constexpr static std::string_view UNKNOWN_KEY = "UnknownKey";

	static std::optional<KeyCode> fromString(std::string_view str) {
		auto it = std::ranges::find_if(entries,
			[str](const Entry& e) { return e.name == str; });
		return (it != std::end(entries)) ? std::make_optional(it->code) : std::nullopt;
	}

	static std::string_view toString(KeyCode code) {
		auto it = std::ranges::find_if(entries,
			[code](const Entry& e) { return e.code == code; });
		return (it != std::end(entries)) ? it->name : UNKNOWN_KEY;
	}

private:
	struct Entry { KeyCode code; std::string_view name; };

	static constexpr Entry entries[] = {
		{ KeyCode::Space,  "SPACE" }
	};
};


class KeyBindingsContext {
public:
	KeyBindingsContext(const std::string& data);
	[[nodiscard]] std::string serialize() const;

	[[nodiscard]] std::optional<ActionCode> translate(KeyCode key) const;

private:
	std::unordered_map<KeyCode, ActionCode> bindings;
};

namespace KeyBindings {
	extern std::unique_ptr<KeyBindingsContext> game;
#if defined(PLATFORM_DESKTOP)
	extern std::unique_ptr<KeyBindingsContext> editor;
#endif

	void load();
}


#endif // KEY_BINDINGS_H