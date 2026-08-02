#include "App.h"

#include "AppMode.h"
#include "AssetManager.h"
#include "FPSOverlay.h"
#include "Settings.h"
#include "Shader.h"
#include <ranges>

void ScreenVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, uv));
	glEnableVertexAttribArray(1);
}

App::App(std::unique_ptr<AbstractWindow> appWindow, std::unique_ptr<AppMode> appMode) : window(std::move(appWindow)), content(std::move(appMode)) {
	auto rootNode = std::make_unique<UINode>();
	auto fps = std::make_unique<FPSOverlay>();
	fps->layout = { .offset = {10.f, 10.f} };
	fpsOverlay = rootNode->addChild(std::move(fps));
	overlayUI.setRootNode(std::move(rootNode));

	quadVertices.emplace_back(glm::vec2(-1, 1), glm::vec2(0, 1));
	quadVertices.emplace_back(glm::vec2(-1, -1), glm::vec2(0, 0));
	quadVertices.emplace_back(glm::vec2(1, -1), glm::vec2(1, 0));
	quadVertices.emplace_back(glm::vec2(1, 1), glm::vec2(1, 1));

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

	content->resize(window->config.width, window->config.height, window->config.dpiScale);
	overlayUI.resize(window->config.width, window->config.height, window->config.dpiScale);
}

void App::tick(microseconds dt) {
	glViewport(0, 0, window->config.width, window->config.height);
	glScissor(0, 0, window->config.width, window->config.height);

	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (content)
		content->tick(dt);

	overlayUI.update(dt);

	overlayUI.render();
}


void App::resizeWindow() {
	window->updateWindowSize();
	content->resize(window->config.width, window->config.height, window->config.dpiScale);
	overlayUI.resize(window->config.width, window->config.height, window->config.dpiScale);
}

void App::updateDPIScale() {
	window->updateWindowDPIScale();
	content->resize(window->config.width, window->config.height, window->config.dpiScale);
	overlayUI.resize(window->config.width, window->config.height, window->config.dpiScale);
}


void App::processEvent(const Event& event) {
	if (overlayUI.processEvent(event))
		return;

	if (auto* key = std::get_if<KeyEvent>(&event)) {
		if (auto actionCode = Settings::Bindings->translate(key->chord)) {
			if (key->action == KeyAction::Down) {
				switch (*actionCode) {
				case ActionCode::Quit:
					window->close();
					return;
				case ActionCode::Fullscreen:
					window->toggleFullscreen();
					return;
				default:;
				}
			}
		}
	}

	content->processEvent(event);
}