#ifndef APP_H
#define APP_H

#include "ui/UIManager.h"

struct AbstractWindow;
class AppMode;
class FPSOverlay;


struct ScreenVertex {
	glm::vec2 pos;
	glm::vec2 uv;

	ScreenVertex() = default;
	ScreenVertex(glm::vec2 pos, glm::vec2 uv) : pos(pos), uv(uv) {}

	static void setupLayout();
};



class App {
public:
	explicit App(std::unique_ptr<AbstractWindow> appWindow);

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	bool tick(microseconds dt);

	void resizeWindow();
	void updateDPIScale();

	void processEvent(const Event& event);
	void requestQuit();

	std::unique_ptr<AbstractWindow> window;

private:
	bool quit = false;

	std::unique_ptr<AppMode> content;

	UIManager overlayUI;
	FPSOverlay* fpsOverlay = nullptr;

	std::unique_ptr<Mesh<ScreenVertex>> quadMesh;
	std::vector<ScreenVertex> quadVertices;
	std::vector<Index> quadIndices;
};

#endif // APP_H
