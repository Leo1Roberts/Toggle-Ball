#ifndef TRANSLATE_OPERATION_H
#define TRANSLATE_OPERATION_H

#include "TransformOperation.h"
#include "editor/GizmoRenderer.h"


class TranslateOperation : public TransformOperation {
public:
	TranslateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: TransformOperation(context, trigger, initialPointerPosition) { TranslateOperation::createUI(); }
	explicit TranslateOperation(const TransformOperation& other)
		: TransformOperation(other) { TranslateOperation::createUI(); }

	static std::optional<glm::vec2> keyToTranslationVector(KeyCode key);

	void renderGizmos() override;

protected:
	void createUI() override;
	void updateDetailsText() override;

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	void setConstraint(glm::vec2 requestedAxis);

	enum class ConstraintType { None, GlobalAxis, LocalAxis };

	ConstraintType constraint = ConstraintType::None;
	glm::vec2 baseAxis{};
	glm::vec2 rawTranslation{0.f};

	glm::vec2 translation{0.f};
	float magnitude = 0.f;

	InfiniteLine focusedAxisLine{};
	std::vector<InfiniteLine> otherAxisLines;
};


#endif // TRANSLATE_OPERATION_H
