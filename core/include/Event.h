#ifndef EVENT_H
#define EVENT_H

#include "Input.h"
#include "KeyBindings.h"
#include <glm/glm.hpp>

#include <variant>


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };


struct KeyEvent { KeyChord chord; KeyAction action; };
struct CharEvent { unsigned int character; };
struct PointerEvent {
	int id{};
	glm::vec2 position;
	PointerAction action{};
	PointerButton button{};
	byte modifiers{};
	glm::vec2 scroll;
};

using Event = std::variant<
	KeyEvent,
	CharEvent,
	PointerEvent
>;

#endif // EVENT_H