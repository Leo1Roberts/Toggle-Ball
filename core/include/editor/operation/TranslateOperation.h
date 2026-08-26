#ifndef TRANSLATE_OPERATION_H
#define TRANSLATE_OPERATION_H

#include "TransformOperation.h"
#include "editor/GizmoRenderer.h"


class TranslateOperation : public TransformOperation {
public:
	TranslateOperation(EditorScene& scene, const Camera& camera, const TransformQuickSettings& settings, TriggerType trigger, glm::vec2 initialPlanarPosition = {})
		: TransformOperation(scene, camera, settings, trigger, initialPlanarPosition) { canStartTyping = false; }
	explicit TranslateOperation(const TransformOperation& other)
		: TransformOperation(other) {}

	static std::optional<glm::vec2> keyToTranslationVector(KeyCode key);

	[[nodiscard]] std::vector<BindingHint> getBindingHints() const override;
	bool updateUI() override;
	void renderGizmos(GizmoRenderer& gizmoRenderer) override;

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;
	void applyOperation() override;

private:
	void updateTransformation(glm::vec2 newPointerPlanarPosition) override {
		rawTranslation += (newPointerPlanarPosition - pointerPlanarPosition) * precisionMultiplier;
	}

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
