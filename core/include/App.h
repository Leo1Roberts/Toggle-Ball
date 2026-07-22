#ifndef APP_H
#define APP_H

#include "EditorMode.h"
#include "Event.h"
#include "GameMode.h"
#include "IWindow.h"
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
	explicit App(IWindow* window);

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	void processEvent(const Event& event);

	void tick(microseconds dt, int width, int height, float dpi);

private:
	IWindow* window;

	int windowWidth{}, windowHeight{};
	float windowDPI{};

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
