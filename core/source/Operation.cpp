#include "Operation.h"

#include "EditorContext.h"
#include "EditorScene.h"


bool Operation::processEvent(const Event& event, EditorContext& editor) {
	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (key->action == KeyAction::Down) {
			if (key->chord.code == KeyCode::Escape) {
				editor.scene->cancelOperation();
				return true;
			}
		}
	}

	return doProcessEvent(event, editor);
}