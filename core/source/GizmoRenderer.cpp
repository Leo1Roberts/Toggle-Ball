#include "GizmoRenderer.h"

#include "Camera.h"
#include "SelectBox.h"
#include "UIManager.h"


void GizmoRenderer::drawBox(const SelectBox& box, const PanelStyle& style) {
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

	panelRenderer.flush(ui->getProjectionMatrix());
}