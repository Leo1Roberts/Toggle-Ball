#include "editor/GizmoRenderer.h"

#include "utilities/Camera.h"
#include "editor/SelectBox.h"


void GizmoRenderer::addBox(const SelectBox& box, const PanelStyle& style) {
	float physicalToLogical = 1.f / ui->getScale();
	glm::vec2 boundsPosition = camera->planarToScreenPosition(glm::vec2(box.left, box.top)) * physicalToLogical;
	glm::vec2 boundsOppositePosition = camera->planarToScreenPosition(glm::vec2(box.right, box.bottom)) * physicalToLogical;
	glm::vec2 boundsSize = boundsOppositePosition - boundsPosition;

	Rectangle bounds = {
		.position = boundsPosition,
		.size = boundsSize,
	};

	auto panel = UIPanel(style);
	panel.setAbsoluteBounds(bounds);

	panelRenderer.addPanel(&panel);
}


void GizmoRenderer::addLine(InfiniteLine line, const LineStyle& style) {
	line.point = camera->planarToScreenPosition(line.point) / ui->getScale();
	line.direction = normalize(Camera::planarToScreenDirection(line.direction));

	glm::vec2 screenSize = ui->getLogicalScreenSize();

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