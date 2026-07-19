#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>


struct PointerEvent;


enum class KeyAction {
	Down,
	Repeat,
	Up,
	Unknown
};

enum class PointerAction {
	Down,
	Move,
	Up,
	Scroll
};
enum class PointerButton {
	Primary,
	Secondary,
	Tertiary
};


struct PointerState {
	glm::vec2 position;
	std::unordered_set<PointerButton> activeButtons;

	[[nodiscard]] bool isDown(PointerButton button) const {
		return activeButtons.contains(button);
	}
};

class InputState {
public:
	void updateFromEvent(const PointerEvent& event);

	[[nodiscard]] const PointerState* getPointer(int id) const {
		auto it = pointers.find(id);
		return it != pointers.end() ? &it->second : nullptr;
	}

private:
	std::unordered_map<int, PointerState> pointers;
};


#endif // INPUT_STATE_H
