#ifndef SHAPE_MODE_H
#define SHAPE_MODE_H

#include "ToolMode.h"


class ShapeMode : public ToolMode {
public:
	explicit ShapeMode(EditorScene* scene, const Camera* camera)
		: ToolMode(scene, camera) {}
	
private:
	[[nodiscard]] ToolModeResponse doProcessEvent(const Event& event) override;

	std::unique_ptr<Operation> startDrag(const PointerEvent& dragStartEvent) override;

	float minorRadius = 0.7f;
};


#endif // SHAPE_MODE_H
