#ifndef UI_PANEL_H
#define UI_PANEL_H

#include "UINode.h"
#include "Mesh.h"
#include "UIStyle.h"


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


class UIPanel : public UINode {
public:
	explicit UIPanel(const PanelStyle& style = {}) : panelStyle(style) {}

	void submitRender(UIManager& manager) override;

	PanelStyle panelStyle;

protected:
	// Only call from within contains(), as it may assume the basic bounds check has already been made
	[[nodiscard]] bool containsPrecise(glm::vec2 point) const override;
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
