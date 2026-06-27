#ifndef EDITOR_H
#define EDITOR_H

enum class SelectionType {
	Replace,
	Add,
	Subtract
};

struct SelectBox {
	float top, bottom, left, right;
	SelectionType selectionType;
};

class Editor {

};

#endif // EDITOR_H
