#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

#include "AppMode.h"
#include "EditorScreen.h"


class EditorMode : public AppMode {
public:
	EditorMode();

private:
	std::unique_ptr<EditorScreen> editorScreen;
};


#endif // EDITOR_MODE_H
