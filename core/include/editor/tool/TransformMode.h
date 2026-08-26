#ifndef TRANSFORM_MODE_H
#define TRANSFORM_MODE_H

#include "ToolMode.h"
#include "editor/operation/TransformOperation.h"


class TransformMode : public ToolMode {
public:
	explicit TransformMode(EditorScene& scene, const Camera& camera)
		: ToolMode(scene, camera) {}

private:
	std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) override;
};


#endif // TRANSFORM_MODE_H
