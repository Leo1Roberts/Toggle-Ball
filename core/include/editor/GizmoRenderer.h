#ifndef GIZMO_RENDERER_H
#define GIZMO_RENDERER_H

#include "ui/UIManager.h"
#include "ui/UIPanel.h"


struct SelectBox;
class Camera;

struct InfiniteLine {
	glm::vec2 point;
	glm::vec2 direction;
};

class GizmoRenderer {
public:
	GizmoRenderer(const UIManager& ui, const Camera& camera)
		: ui(ui), camera(camera) {}

	// All coordinates given in world space

	void addBox(const SelectBox& box, const PanelStyle& style);
	void addLine(glm::vec2 p1, glm::vec2 p2, const LineStyle& style);
	void addLine(InfiniteLine line, const LineStyle& style);

	void render() { panelRenderer.flush(ui.getProjectionMatrix()); }

private:
	const UIManager& ui;
	const Camera& camera;

	UIPanelRenderer panelRenderer;
};


#endif // GIZMO_RENDERER_H
