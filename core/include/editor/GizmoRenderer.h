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
	void addCircle(glm::vec2 centre, const PanelStyle& style);
	void addSplitCircle(glm::vec2 centre, float radius, float style1CentreAngle, const PanelStyle& style1, const PanelStyle& style2);
	void addLine(glm::vec2 p1, glm::vec2 p2, const LineStyle& style);
	void addLine(InfiniteLine line, const LineStyle& style);

	void render() { panelRenderer.flush(ui.getProjectionMatrix()); }

	[[nodiscard]] float planarToUIDistance(float planarDistance) const;
	[[nodiscard]] float uiToPlanarDistance(float uiDistance) const;

private:
	[[nodiscard]] glm::vec2 planarToUIPosition(glm::vec2 planarPosition) const;

	const UIManager& ui;
	const Camera& camera;

	UIPanelRenderer panelRenderer;
};

struct IGizmoProvider {
	virtual ~IGizmoProvider() = default;

	virtual void addGizmos(GizmoRenderer& gizmoRenderer) const {}
};


#endif // GIZMO_RENDERER_H
