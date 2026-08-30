#include "ui/UIPanel.h"
#include "opengl/Shader.h"
#include "ui/UIManager.h"


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


UIResponse UIPanel::processEvent(const Event& event) {
	if (auto pointer = std::get_if<PointerEvent>(&event)) {
		switch (pointer->action) {
		case PointerAction::Down:
		case PointerAction::Up:
			return UIResponse::Consumed;
		default:;
		}
	}

	return UIResponse::Ignored;
}


void UIPanel::submitRender(UIManager& manager) {
	manager.submitPanel(this);
}


bool UIPanel::containsPrecise(glm::vec2 point) const {
	const auto& bounds = getAbsoluteBounds();

	if (panelStyle.cornerRadius == 0.f)
		return true;

	float halfWidth = bounds.width() * 0.5f;
	float halfHeight = bounds.height() * 0.5f;

	float centerX = bounds.x() + halfWidth;
	float centerY = bounds.y() + halfHeight;

	float dx = std::abs(point.x - centerX);
	float dy = std::abs(point.y - centerY);

	float cornerRadius = std::min(panelStyle.cornerRadius, bounds.height());

	float circleCenterX = halfWidth - cornerRadius;
	float circleCenterY = halfHeight - cornerRadius;

	if (dx <= circleCenterX || dy <= circleCenterY)
		return true;

	float cornerDx = dx - circleCenterX;
	float cornerDy = dy - circleCenterY;

	return (cornerDx * cornerDx + cornerDy * cornerDy) <= (cornerRadius * cornerRadius);
}



void UIPanelRenderer::addPanel(const UIPanel* panel) {
	const auto& bounds = panel->getAbsoluteBounds();

	float
	l = bounds.x(),
	r = bounds.x() + bounds.width(),
	b = bounds.y(),
	t = bounds.y() + bounds.height();

	float maxFeatureSize = std::min(bounds.width(), bounds.height()) * 0.5f;
	float strokeWidth = std::max(0.f, std::min(panel->panelStyle.strokeWidth, maxFeatureSize));
	float rad = std::max(0.f, std::min(panel->panelStyle.cornerRadius, maxFeatureSize));

	float strokeRad, inset;
	if (rad > strokeWidth) {
		strokeRad = (rad - strokeWidth) / rad;
		inset = 0.f;
	} else {
		strokeRad = 0.f;
		inset = strokeWidth - rad;
	}

	Index base = vertices.size();

	// Top row
	vertices.emplace_back(glm::vec2(l, t),             glm::vec2(1.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(l + rad, t),       glm::vec2(0.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad, t),       glm::vec2(0.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r, t),             glm::vec2(1.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	// Second row
	vertices.emplace_back(glm::vec2(l, t - rad),       glm::vec2(1.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(l + rad, t - rad), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad, t - rad), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r, t - rad),       glm::vec2(1.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	// Third row
	vertices.emplace_back(glm::vec2(l, b + rad),       glm::vec2(1.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(l + rad, b + rad), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad, b + rad), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r, b + rad),       glm::vec2(1.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	// Bottom row
	vertices.emplace_back(glm::vec2(l, b),             glm::vec2(1.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(l + rad, b),       glm::vec2(0.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad, b),       glm::vec2(0.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r, b),             glm::vec2(1.f, 1.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	// Inset centre quad
	vertices.emplace_back(glm::vec2(l + rad + inset, t - rad - inset), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(l + rad + inset, b + rad + inset), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad - inset, b + rad + inset), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad - inset, t - rad - inset), glm::vec2(0.f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	// Inset centre quad (for joining to bordering quads)
	vertices.emplace_back(glm::vec2(l + rad + inset, t - rad - inset), glm::vec2(0.005f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(l + rad + inset, b + rad + inset), glm::vec2(0.005f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad - inset, b + rad + inset), glm::vec2(0.005f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);
	vertices.emplace_back(glm::vec2(r - rad - inset, t - rad - inset), glm::vec2(0.005f, 0.f), panel->panelStyle.fillColor, panel->panelStyle.strokeColor, strokeRad);

	// Base quads
	for (int i = 0; i < 9; i++) {
		if (i == 4) {
			indices.push_back(base + 16 + 0); indices.push_back(base + 16 + 1); indices.push_back(base + 16 + 2);
			indices.push_back(base + 16 + 0); indices.push_back(base + 16 + 2); indices.push_back(base + 16 + 3);
		} else {
			Index quadBase = base + (i / 3 * 4) + (i % 3);
			indices.push_back(quadBase);	  indices.push_back(quadBase + 4);  indices.push_back(quadBase + 5);
			indices.push_back(quadBase);	  indices.push_back(quadBase + 5);  indices.push_back(quadBase + 1);
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

void UIPanelRenderer::addCircle(glm::vec2 centre, const PanelStyle& style) {
	float strokeRad = std::clamp((style.cornerRadius - style.strokeWidth) / style.cornerRadius, 0.f, 1.f);

	Index base = vertices.size();

	vertices.emplace_back(centre + glm::vec2(-style.cornerRadius,  style.cornerRadius), glm::vec2(-1.f,  1.f), style.fillColor, style.strokeColor, strokeRad);
	vertices.emplace_back(centre + glm::vec2( style.cornerRadius,  style.cornerRadius), glm::vec2( 1.f,  1.f), style.fillColor, style.strokeColor, strokeRad);
	vertices.emplace_back(centre + glm::vec2( style.cornerRadius, -style.cornerRadius), glm::vec2( 1.f, -1.f), style.fillColor, style.strokeColor, strokeRad);
	vertices.emplace_back(centre + glm::vec2(-style.cornerRadius, -style.cornerRadius), glm::vec2(-1.f, -1.f), style.fillColor, style.strokeColor, strokeRad);

	indices.push_back(base + 0);
	indices.push_back(base + 1);
	indices.push_back(base + 2);
	indices.push_back(base + 0);
	indices.push_back(base + 2);
	indices.push_back(base + 3);
}

void UIPanelRenderer::addLine(glm::vec2 p1, glm::vec2 p2, const LineStyle& style) {
	glm::vec2 diff = p2 - p1;
	float length = glm::length(diff);

	if (length <= 0.0001f)
		return;

	glm::vec2 dir = diff / length;
	glm::vec2 normal = glm::vec2(-dir.y, dir.x);
	glm::vec2 offset = normal * (style.width * 0.5f);

	auto addSegment = [&](glm::vec2 start, glm::vec2 end, col c) {
		glm::vec2 v0 = start - offset;
		glm::vec2 v1 = start + offset;
		glm::vec2 v2 = end + offset;
		glm::vec2 v3 = end - offset;

		Index base = vertices.size();

		vertices.emplace_back(v0, glm::vec2(0.f, 0.f), c, c, 0.f);
		vertices.emplace_back(v1, glm::vec2(0.f, 0.f), c, c, 0.f);
		vertices.emplace_back(v2, glm::vec2(0.f, 0.f), c, c, 0.f);
		vertices.emplace_back(v3, glm::vec2(0.f, 0.f), c, c, 0.f);

		indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
		indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
	};

	if (style.dashLength > 0.f) {
		glm::vec2 centre = p1 + dir * (length * 0.5f);
		float halfLength = length * 0.5f;

		float current_t = 0.f;
		int step = 0;

		while (current_t < halfLength) {
			float next_t = (step == 0) ? (0.5f * style.dashLength) : (current_t + style.dashLength);
			next_t = std::min(next_t, halfLength);

			col color = (step % 2 == 0) ? style.secondaryColor : style.primaryColor;

			// Add segment 'in front of' centre
			glm::vec2 f_start = centre + dir * current_t;
			glm::vec2 f_end   = centre + dir * next_t;
			addSegment(f_start, f_end, color);

			// Add segment 'behind' centre
			glm::vec2 b_start = centre - dir * next_t;
			glm::vec2 b_end   = centre - dir * current_t;
			addSegment(b_start, b_end, color);

			current_t = next_t;
			step++;
		}
	} else {
		// Fallback to solid color1 if dashLength is 0
		addSegment(p1, p2, style.primaryColor);
	}
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