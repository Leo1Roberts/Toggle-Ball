#ifndef TRANSFORM_OPERATION_H
#define TRANSFORM_OPERATION_H

#include "editor/EditorScene.h"
#include "Operation.h"
#include "io/TextInputBuffer.h"

class UIText;


struct TransformQuickSettings {
	bool transformBothStates = false;
	bool transformIndividually = false;
};


class TransformOperation : public Operation {
public:
	TransformOperation(EditorScene* scene, const Camera* camera, const TransformQuickSettings& settings, TriggerType trigger, glm::vec2 initialPointerPosition)
		: Operation(scene, camera, trigger, initialPointerPosition), settings(settings), pointerPlanarPosition(initialPointerPlanarPosition) {}
	TransformOperation(const TransformOperation& other) // Reset typing
		: Operation(other), settings(other.settings), pointerPlanarPosition(initialPointerPlanarPosition) {}

	[[nodiscard]] static std::unique_ptr<TransformOperation> make(ActionCode actionCode, EditorScene* scene, const Camera* camera, const TransformQuickSettings& settings, TriggerType trigger, glm::vec2 initialPointerPosition);
	[[nodiscard]] static std::unique_ptr<TransformOperation> makeFromExisting(ActionCode actionCode, const TransformOperation* existingOperation);

	[[nodiscard]] std::vector<BindingHint> getBindingHints() const override;
	void createUI(UINode& container) override;
	bool updateUI() override;

	void cancel() const final { return scene->cancelLevelChange(); }
	void commit() const final { return scene->commitLevelChange(); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	const TransformQuickSettings& settings;

	glm::vec2 pointerPlanarPosition;

	float precisionMultiplier = 1.f;

	bool typing = false;
	TextInputBuffer textInput{TextInputBuffer::Float, TextInputMode::Simple};

	UIText* detailsText = nullptr;

private:
	virtual void updateTransformation(glm::vec2 newPointerPlanarPosition) = 0;

	void applyModifiers(byte mods) final {
		precisionMultiplier = mods & MOD_SHIFT ? 0.1f : 1.f;
	}

	[[nodiscard]] bool canStart() const final { return scene->anythingIsSelected(); }
};


#endif // TRANSFORM_OPERATION_H
