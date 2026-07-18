#ifndef EVENT_H
#define EVENT_H

#include "Input.h"
#include "KeyBindings.h"

#include <variant>


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };


struct KeyEvent { KeyCode code; KeyAction action; };
struct CharEvent { unsigned int character; };
struct PointerEvent {
	int id{};
	vec2 position;
	PointerAction action{};
	PointerButton button{};
	vec2 scroll;
};

using Event = std::variant<
	KeyEvent,
	CharEvent,
	PointerEvent
>;

#endif // EVENT_H