#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

#include "AppMode.h"
#include "EditorEntryScreen.h"
#include "EditorScreen.h"
#include "PlayTestScreen.h"


class EditorMode : public AppMode {
public:
	EditorMode(const std::function<void()>& quitCallback);

	void tick(microseconds dt) override;
	void resize(int windowWidth, int windowHeight, float windowDPI) override;
	void processEvent(const Event& event) override;
	bool requestQuit() override;
	[[nodiscard]] std::optional<Cursor> queryCursor() const override;

private:
	void startEditing(const std::string& levelName);
	void resumeEditing();
	void testLevel();
	void openEntryScreen();

	std::function<void()> scheduledScreenChange;

	std::unique_ptr<EditorEntryScreen> editorEntryScreen;
	std::unique_ptr<EditorScreen> editorScreen;
	std::unique_ptr<PlayTestScreen> playTestScreen;

	const std::function<void()>& quitCallback;

	UIManager uiManager;
	UIDialogue* confirmQuitDialogue;
};


#endif // EDITOR_MODE_H
