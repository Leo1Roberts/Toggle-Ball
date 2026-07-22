#include "UIPanel.h"
#include "Shader.h"
#include "UIManager.h"


void UIPanelVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIPanelVertex), (void*)offsetof(UIPanelVertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIPanelVertex), (void*)offsetof(UIPanelVertex, uv));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(UIPanelVertex), (void*)offsetof(UIPanelVertex, fillColor));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(UIPanelVertex), (void*)offsetof(UIPanelVertex, strokeColor));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(UIPanelVertex), (void*)offsetof(UIPanelVertex, strokeRadius));
	glEnableVertexAttribArray(4);
}



void UIPanel::submitRender(UIManager& manager) {
	manager.submitPanel(this);
}


bool UIPanel::containsPrecise(glm::vec2 point) const {
	const auto& bounds = getAbsoluteBounds();

	if (panelStyle.cornerRadius == 0.f)
		return true;

	float halfWidth = bounds.width * 0.5f;
	float halfHeight = bounds.height * 0.5f;

	float centerX = bounds.x + halfWidth;
	float centerY = bounds.y + halfHeight;

	float dx = std::abs(point.x - centerX);
	float dy = std::abs(point.y - centerY);

	float circleCenterX = halfWidth - panelStyle.cornerRadius;
	float circleCenterY = halfHeight - panelStyle.cornerRadius;

	if (dx <= circleCenterX || dy <= circleCenterY)
		return true;

	float cornerDx = dx - circleCenterX;
	float cornerDy = dy - circleCenterY;

	return (cornerDx * cornerDx + cornerDy * cornerDy) <= (panelStyle.cornerRadius * panelStyle.cornerRadius);
}



void UIPanelRenderer::addPanel(const UIPanel* panel) {
	const auto& bounds = panel->getAbsoluteBounds();

	float
	l = bounds.x,
	r = bounds.x + bounds.width,
	b = bounds.y,
	t = bounds.y + bounds.height;

	const float CS = panel->panelStyle.cornerRadius;
	const float outlineThickness = panel->panelStyle.strokeWidth;

	float outlineRad, inset;
	if (CS > outlineThickness) {
		outlineRad = (CS - outlineThickness) / CS;
		inset = 0;
	} else {
		outlineRad = 0;
		inset = outlineThickness - CS;
	}

	Index base = vertices.size();

	// Top row
	vertices.emplace_back(glm::vec2(l, t),            glm::vec2(1, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(l + CS, t),       glm::vec2(0, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r - CS, t),       glm::vec2(0, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r, t),            glm::vec2(1, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	// Second row
	vertices.emplace_back(glm::vec2(l, t - CS),       glm::vec2(1, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(l + CS, t - CS),  glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r - CS, t - CS),  glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r, t - CS),       glm::vec2(1, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	// Third row
	vertices.emplace_back(glm::vec2(l, b + CS),       glm::vec2(1, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(l + CS, b + CS),  glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r - CS, b + CS),  glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r, b + CS),       glm::vec2(1, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	// Bottom row
	vertices.emplace_back(glm::vec2(l, b),            glm::vec2(1, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(l + CS, b),       glm::vec2(0, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r - CS, b),       glm::vec2(0, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	vertices.emplace_back(glm::vec2(r, b),            glm::vec2(1, 1), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad);
	// Inset centre quad
	vertices.emplace_back(glm::vec2(l + CS + inset, t - CS - inset), glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 16: TL
	vertices.emplace_back(glm::vec2(l + CS + inset, b + CS + inset), glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 17: BL
	vertices.emplace_back(glm::vec2(r - CS - inset, b + CS + inset), glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 18: BR
	vertices.emplace_back(glm::vec2(r - CS - inset, t - CS - inset), glm::vec2(0, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 19: TR
	// Inset centre quad (for joining to bordering quads)
	vertices.emplace_back(glm::vec2(l + CS + inset, t - CS - inset), glm::vec2(0.005f, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 20: TL
	vertices.emplace_back(glm::vec2(l + CS + inset, b + CS + inset), glm::vec2(0.005f, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 21: BL
	vertices.emplace_back(glm::vec2(r - CS - inset, b + CS + inset), glm::vec2(0.005f, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 22: BR
	vertices.emplace_back(glm::vec2(r - CS - inset, t - CS - inset), glm::vec2(0.005f, 0), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, outlineRad); // 23: TR

	// Base quads
	for (int i = 0; i < 9; i++) {
		if (i == 4) {
			indices.push_back(base + 16 + 0); indices.push_back(base + 16 + 1); indices.push_back(base + 16 + 2);
			indices.push_back(base + 16 + 0); indices.push_back(base + 16 + 2); indices.push_back(base + 16 + 3);
		} else {
			Index quadBase = base + (i / 3 * 4) + (i % 3);
			indices.push_back(quadBase);      indices.push_back(quadBase + 4);  indices.push_back(quadBase + 5);
			indices.push_back(quadBase);      indices.push_back(quadBase + 5);  indices.push_back(quadBase + 1);
		}
	}

	// Trapezium insetting quads
	// Top
	indices.push_back(base +  5); indices.push_back(base + 20); indices.push_back(base + 23);
	indices.push_back(base +  5); indices.push_back(base + 23); indices.push_back(base +  6);
	// Left
	indices.push_back(base +  5); indices.push_back(base +  9); indices.push_back(base + 21);
	indices.push_back(base +  5); indices.push_back(base + 21); indices.push_back(base + 20);
	// Bottom
	indices.push_back(base + 21); indices.push_back(base +  9); indices.push_back(base + 10);
	indices.push_back(base + 21); indices.push_back(base + 10); indices.push_back(base + 22);
	// Right
	indices.push_back(base + 23); indices.push_back(base + 22); indices.push_back(base + 10);
	indices.push_back(base + 23); indices.push_back(base + 10); indices.push_back(base +  6);
}


void UIPanelRenderer::flush(const glm::mat4& projectionMatrix) {
	if (vertices.empty()) return;

	Shaders::panel->use();
	Shaders::panel->setMat4("uProjection2D", projectionMatrix);

	mesh->setData(vertices, indices);
	mesh->draw();

	vertices.clear();
	indices.clear();
}