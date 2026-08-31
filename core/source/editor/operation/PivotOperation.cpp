#include "editor/operation/PivotOperation.h"

#include "editor/GizmoRenderer.h"


void PivotOperation::init() {
	auto focus = ctx.scene.selectionFocus;
	if (focus.type == EntityType::Ball)
		pivot = ctx.scene.ball.descriptor->initialPosition;
	else if (focus.type == EntityType::Obstacle)
		pivot = worldToPlanar(ctx.scene.obstacles[focus.index].getKinematicState()->getPosition());
}


void PivotOperation::addGizmos(GizmoRenderer& gizmoRenderer) const {
	if (typing) return;

	gizmoRenderer.addLine(pivot, pointerPlanarPosition, {
		.primaryColor = Color::PointerConnectorLine1,
		.secondaryColor = Color::PointerConnectorLine2,
		.width = Settings::Sizes.lineWidth,
		.dashLength = Settings::Sizes.lineWidth * 3.f,
	});
}