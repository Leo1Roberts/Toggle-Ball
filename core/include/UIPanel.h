#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "UINode.h"
#include "Colors.h"
#include "Mesh.h"


struct UIPanelVertex {
	glm::vec2 position;
	glm::vec2 uv;
	col fillColor;
	col strokeColor;
	float strokeRadius{};

	UIPanelVertex(glm::vec2 position, glm::vec2 uv, col fillColor, col strokeColor, float strokeRadius) :
	    position(position), uv(uv), fillColor(fillColor), strokeColor(strokeColor), strokeRadius(strokeRadius) {}

	static void setupLayout();
};


class UIManager;

class UIPanel : public UINode {
public:
	void submitRender(UIManager& manager) override;

	col fillColor;
	col strokeColor;
	float cornerRadius;
	float strokeWidth;
};


class UIPanelRenderer : public IUIRenderer {
public:
	void addPanel(const UIPanel* panel);

	void flush(const glm::mat4& projectionMatrix) override;

private:
	std::vector<UIPanelVertex> vertices;
	std::vector<Index> indices;
	std::unique_ptr<Mesh<UIPanelVertex>> mesh = std::make_unique<Mesh<UIPanelVertex>>(GL_DYNAMIC_DRAW);
};


#endif // UI_PANEL_H
