#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

#include "AppMode.h"
#include "EditorEntryScreen.h"
#include "EditorScreen.h"
#include "PlayTestScreen.h"


class EditorMode : public AppMode {
public:
	EditorMode() { openEntryScreen(); }

	void processEvent(const Event& event) override;
	bool requestQuit(const std::function<void()>& quitCallback) override;

private:
	void startEditing(const std::string& levelName);
	void resumeEditing();
	void testLevel();
	void openEntryScreen();

	std::unique_ptr<EditorEntryScreen> editorEntryScreen;
	std::unique_ptr<EditorScreen> editorScreen;
	std::unique_ptr<PlayTestScreen> playTestScreen;
};


#endif // EDITOR_MODE_H
