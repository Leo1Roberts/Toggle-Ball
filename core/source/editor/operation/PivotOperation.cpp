#include "editor/operation/PivotOperation.h"

#include "editor/GizmoRenderer.h"


void PivotOperation::init() {
	pointerPlanarPosition = initialPointerPlanarPosition;

	auto focus = context->scene->selectionFocus;
	if (focus.type == EntityType::Ball)
		pivot = context->scene->ball.descriptor->initialPosition;
	else if (focus.type == EntityType::Obstacle)
		pivot = worldToPlanar(context->scene->obstacles[focus.index].getKinematicState()->getPosition());
}


void PivotOperation::renderGizmos() {
	if (typing) return;

	context->gizmoRenderer->addLine(pivot, pointerPlanarPosition, {
		.primaryColor = Color::PointerConnectorLine1,
		.secondaryColor = Color::PointerConnectorLine2,
		.width = Settings::Sizes.lineWidth,
		.dashLength = Settings::Sizes.lineWidth * 3.f,
	});

	context->gizmoRenderer->render();
}