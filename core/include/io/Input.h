#ifndef INPUT_STATE_H
#define INPUT_STATE_H

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
    Leave,

	Scroll,

	StartDrag,
	Drag,
	FinishDrag,
	CancelDrag,

	Unknown
};
enum class PointerButton {
	Primary,
	Secondary,
	Tertiary,
	Unknown
};


#endif // INPUT_STATE_H
