#ifndef TRANSFORM_OPERATION_H
#define TRANSFORM_OPERATION_H

#include "editor/EditorScene.h"
#include "Operation.h"
#include "io/TextInputBuffer.h"

class UIText;


class TransformOperation : public Operation {
public:
	TransformOperation(EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition)
		: Operation(scene, camera, quickSettings, trigger, initialPlanarPosition) {}
	TransformOperation(const TransformOperation& other) // Reset typing
		: Operation(other) {}

	[[nodiscard]] static std::unique_ptr<TransformOperation> make(ActionCode actionCode, EditorScene& scene, const Camera& camera, const EditorQuickSettings& quickSettings, TriggerType trigger, glm::vec2 initialPlanarPosition);
	[[nodiscard]] static std::unique_ptr<TransformOperation> makeFromExisting(ActionCode actionCode, const TransformOperation* existingOperation);

	[[nodiscard]] std::vector<BindingHint> getBindingHints() const override;
	void createUI(UINode& container) override;
	bool updateUI() override;

	void cancel() const final { scene.cancelLevelChange(); }
	void commit() const final { scene.commitLevelChange(); }

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;

	float precisionMultiplier = 1.f;

	bool canStartTyping = true;
	bool typing = false;
	TextInputBuffer textInput{TextInputBuffer::Float, TextInputMode::Simple};

	UIText* detailsText = nullptr;

private:
	virtual void updateTransformation(glm::vec2 newPointerPlanarPosition) = 0;

	void applyModifiers(byte mods) final {
		precisionMultiplier = mods & MOD_SHIFT ? 0.1f : 1.f;
	}

	[[nodiscard]] bool canStart() const final { return scene.anythingIsSelected(); }
};


#endif // TRANSFORM_OPERATION_H
