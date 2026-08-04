#ifndef GIZMO_RENDERER_H
#define GIZMO_RENDERER_H

#include "ui/UIPanel.h"


struct SelectBox;
class Camera;

class GizmoRenderer {
public:
	GizmoRenderer(const UIManager* ui, const Camera* camera)
		: ui(ui), camera(camera) {}

	// Box coordinates given in world space
	void drawBox(const SelectBox& box, const PanelStyle& style);

private:
	const UIManager* ui;
	const Camera* camera;

	UIPanelRenderer panelRenderer;
};


#endif // GIZMO_RENDERER_H
