#ifndef EVENT_H
#define EVENT_H

#include "Input.h"
#include "KeyBindings.h"
#include <glm/glm.hpp>

#include <variant>


struct KeyEvent { KeyChord chord; KeyAction action; };
struct PointerEvent {
	int id{};
	glm::vec2 position;
	PointerAction action{};
	PointerButton button{};
	byte modifiers{};
	glm::vec2 scroll;
	bool causedFocusChange = false;
	int pointerDownCount = 0;
};

using Event = std::variant<
	KeyEvent,
	char,
	PointerEvent
>;

#endif // EVENT_H