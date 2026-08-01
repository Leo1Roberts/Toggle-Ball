#ifndef TRANSLATE_OPERATION_H
#define TRANSLATE_OPERATION_H

#include "TransformOperation.h"
#include "Operation.h"


class TranslateOperation : public TransformOperation {
public:
	TranslateOperation(const EditorContext* context, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: TransformOperation(context, trigger, initialPointerPosition) {}

	static std::optional<glm::vec2> keyToTranslationVector(KeyCode key);

private:
	bool doProcessEvent(const Event& event) override;
	void applyOperation() override;

	void setMode(glm::vec2 requestedAxis);

	enum class ConstraintType { None, GlobalAxis, LocalAxis };

	ConstraintType constraint = ConstraintType::None;
	glm::vec2 baseAxis{};
	glm::vec2 rawTranslation{0.f};
};


#endif // TRANSLATE_OPERATION_H
