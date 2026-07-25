#ifndef APP_H
#define APP_H

#include "EditorMode.h"
#include "Event.h"
#include "GameMode.h"
#include "AbstractWindow.h"
#include "Mesh.h"
#include "UIManager.h"


struct ScreenVertex {
	glm::vec2 pos;
	glm::vec2 uv;

	ScreenVertex() = default;
	ScreenVertex(glm::vec2 pos, glm::vec2 uv) : pos(pos), uv(uv) {}

	static void setupLayout();
};


class FPSOverlay;

class App {
public:
	explicit App(std::unique_ptr<AbstractWindow> appWindow);

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	void tick(microseconds dt);

	void resize(int width, int height);
	void updateDPIScale(float dpi);

	void processEvent(const Event& event);

	std::unique_ptr<AbstractWindow> window;

private:
	AppMode* activeMode;
	std::unique_ptr<GameMode> gameMode;
	std::unique_ptr<EditorMode> editorMode;

	UIManager overlayUI;
	FPSOverlay* fpsOverlay = nullptr;

	std::unique_ptr<Mesh<ScreenVertex>> quadMesh;
	std::vector<ScreenVertex> quadVertices;
	std::vector<Index> quadIndices;
};

#endif // APP_H
