#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

#include "AppMode.h"
#include "EditorScreen.h"
#include "PlayTestScreen.h"


class EditorMode : public AppMode {
public:
	EditorMode();

	void processEvent(const Event& event) override;

private:
	void resumeEditing();
	void testLevel();

	std::unique_ptr<EditorScreen> editorScreen;
	std::unique_ptr<PlayTestScreen> playTestScreen;
};


#endif // EDITOR_MODE_H
