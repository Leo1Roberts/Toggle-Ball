#ifndef SCALE_OPERATION_H
#define SCALE_OPERATION_H

#include "PivotOperation.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"


class ScaleOperation : public PivotOperation {
public:
	ScaleOperation(EditorScene* scene, const Camera* camera, const TransformQuickSettings& settings, TriggerType trigger, glm::vec2 initialPointerPosition = {})
		: PivotOperation(scene, camera, settings, trigger, initialPointerPosition) {}
	explicit ScaleOperation(const TransformOperation& other)
		: PivotOperation(other) {}

	[[nodiscard]] std::vector<BindingHint> getBindingHints() const override;
	bool updateUI() override;

protected:
	[[nodiscard]] OperationResponse doProcessEvent(const Event& event) override;
	void applyOperation() override;

private:
	void updateTransformation(glm::vec2 newPointerPlanarPosition) override {
		scale *= std::pow(std::sqrt(length2(newPointerPlanarPosition - pivot) / length2(pointerPlanarPosition - pivot)), precisionMultiplier);
	}

	enum class Dimension : int { MajorAndMinor, Major, Minor, COUNT };

	Dimension dimension = Dimension::MajorAndMinor;
	float scale = 1.f;
};


#endif // SCALE_OPERATION_H
