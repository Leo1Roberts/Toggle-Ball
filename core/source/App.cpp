#include "App.h"
#include "Shader.h"
#include "Texture.h"

#include <ranges>

void ScreenVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, uv));
	glEnableVertexAttribArray(1);
}

App::App() {
	Meshes::load();
	Shaders::load();
	Textures::load();

	quadVertices.emplace_back(vec2(-1, 1), vec2(0, 1));
	quadVertices.emplace_back(vec2(-1, -1), vec2(0, 0));
	quadVertices.emplace_back(vec2(1, -1), vec2(1, 0));
	quadVertices.emplace_back(vec2(1, 1), vec2(1, 1));

	quadIndices.push_back(0);
	quadIndices.push_back(1);
	quadIndices.push_back(2);
	quadIndices.push_back(0);
	quadIndices.push_back(2);
	quadIndices.push_back(3);

	quadMesh = std::make_unique<Mesh<ScreenVertex>>(quadVertices, quadIndices);
	quadMesh->setData(quadVertices, quadIndices);

	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void App::processEvent(const Event& event) {
	for (const auto& screen: std::views::reverse(screens))
		if (screen->processEvent(event)) break;
}

void App::tick(microseconds dt, int windowWidth, int windowHeight) const {
	for (const auto& screen : screens)
		screen->update(dt);

	for (const auto& screen: screens) {
		screen->resize(windowWidth, windowHeight);
		screen->draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glScissor(0, 0, windowWidth, windowHeight);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (const auto& screen: screens) {
		Shaders::quad->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screen->getTexture());
		quadMesh->draw();
	}
}