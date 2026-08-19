#include "editor/operation/PivotOperation.h"

#include "editor/GizmoRenderer.h"


void PivotOperation::init() {
	auto focus = scene->selectionFocus;
	if (focus.type == EntityType::Ball)
		pivot = scene->ball.descriptor->initialPosition;
	else if (focus.type == EntityType::Obstacle)
		pivot = worldToPlanar(scene->obstacles[focus.index].getKinematicState()->getPosition());
}


void PivotOperation::renderGizmos(GizmoRenderer& gizmoRenderer) {
	if (typing) return;

	gizmoRenderer.addLine(pivot, pointerPlanarPosition, {
		.primaryColor = Color::PointerConnectorLine1,
		.secondaryColor = Color::PointerConnectorLine2,
		.width = Settings::Sizes.lineWidth,
		.dashLength = Settings::Sizes.lineWidth * 3.f,
	});

	gizmoRenderer.render();
}