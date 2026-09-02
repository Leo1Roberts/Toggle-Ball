#include "editor/GizmoRenderer.h"

#include "utilities/Camera.h"
#include "editor/SelectBox.h"


void GizmoRenderer::addBox(const SelectBox& box, const PanelStyle& style) {
	glm::vec2 boundsPosition = planarToUIPosition({box.left, box.top});
	glm::vec2 boundsOppositePosition = planarToUIPosition({box.right, box.bottom});
	glm::vec2 boundsSize = boundsOppositePosition - boundsPosition;

	Rectangle bounds = {
		.position = boundsPosition,
		.size = boundsSize,
	};

	auto panel = UIPanel(style);
	panel.setAbsoluteBounds(bounds);

	panelRenderer.addPanel(&panel);
}


void GizmoRenderer::addCircle(glm::vec2 centre, const PanelStyle& style) {
	panelRenderer.addCircle(planarToUIPosition(centre), style);
}
void GizmoRenderer::addSplitCircle(glm::vec2 centre, float radius, float style1CentreAngle, const PanelStyle& style1, const PanelStyle& style2) {
	panelRenderer.addSplitCircle(planarToUIPosition(centre), radius, -style1CentreAngle, style1, style2); // Negate angle to account for negated Y-axis
}


void GizmoRenderer::addLine(glm::vec2 p1, glm::vec2 p2, const LineStyle& style) {
	panelRenderer.addLine(planarToUIPosition(p1), planarToUIPosition(p2), style);
}
void GizmoRenderer::addLine(InfiniteLine line, const LineStyle& style) {
	line.point = camera.planarToScreenPosition(line.point) / ui.getScale();
	line.direction = normalize(Camera::planarToScreenDirection(line.direction));

	glm::vec2 screenSize = ui.getLogicalScreenSize();

	glm::vec2 corners[4] = {
		glm::vec2(0.f),
		glm::vec2(screenSize.x, 0.f),
		screenSize,
		glm::vec2(0.f, screenSize.y)
	};

	float minT = std::numeric_limits<float>::max();
	float maxT = std::numeric_limits<float>::lowest();

	for (auto corner : corners) {
		float t = glm::dot(corner - line.point, line.direction);
		minT = std::min(minT, t);
		maxT = std::max(maxT, t);
	}

	glm::vec2 p1 = line.point + line.direction * minT;
	glm::vec2 p2 = line.point + line.direction * maxT;

	panelRenderer.addLine(p1, p2, style);
}


glm::vec2 GizmoRenderer::planarToUIPosition(glm::vec2 planarPosition) const {
	return camera.planarToScreenPosition(planarPosition) / ui.getScale();
}
float GizmoRenderer::planarToUIDistance(float planarDistance) const {
	return camera.planarToScreenDistance(planarDistance) / ui.getScale();
}
float GizmoRenderer::uiToPlanarDistance(float uiDistance) const {
	return camera.screenToPlanarDistance(uiDistance) * ui.getScale();
}