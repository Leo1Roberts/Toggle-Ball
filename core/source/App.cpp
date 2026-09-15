#include "App.h"

#include "system/AbstractWindow.h"
#include "AppMode.h"
#include "editor/EditorMode.h"
#include "game/GameMode.h"
#include "ui/FPSOverlay.h"


void ScreenVertex::setupLayout() {
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)offsetof(ScreenVertex, uv));
	glEnableVertexAttribArray(1);
}

App::App(std::unique_ptr<AbstractWindow> appWindow) : window(std::move(appWindow)) {
	// auto fps = std::make_unique<FPSOverlay>();
	// fps->setLayout({ .margin = glm::vec2(10.f) });
	// fpsOverlay = overlayUI.addNode(std::move(fps));

#if defined(PLATFORM_ANDROID)
	content = std::make_unique<GameMode>();
#else
	content = std::make_unique<EditorMode>([this] { quit = true; } );
#endif

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

bool App::tick(microseconds dt) {
	if (quit)
		return false;

	glViewport(0, 0, window->config.width, window->config.height);
	glScissor(0, 0, window->config.width, window->config.height);

	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (content)
		content->tick(dt);

	overlayUI.update(dt);

	overlayUI.render();

	std::optional<Cursor> c;
	if (((c = overlayUI.queryCursor())) || ((c = content->queryCursor())))
		window->setCursor(*c);
	else
		window->setCursor();

	return true;
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
					requestQuit();
					return;
				case ActionCode::Fullscreen:
					window->toggleFullscreen();
					return;
				case ActionCode::DecreaseUIScale:
					Settings::Sizes.uiScale = std::max(0.1f, Settings::Sizes.uiScale - 0.1f);
					resizeWindow();
					return;
				case ActionCode::IncreaseUIScale:
					Settings::Sizes.uiScale = std::min(Settings::Sizes.uiScale + 0.1f, 3.f);
					resizeWindow();
					return;
				default:;
				}
			}
		}
	}

	content->processEvent(event);
}

void App::requestQuit() {
	if (content->requestQuit())
		quit = true;
}