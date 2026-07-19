#ifndef APP_H
#define APP_H

#include "Screen.h"
#include "Event.h"
#include "IWindow.h"
#include "Mesh.h"

struct ScreenVertex {
	vec2 pos;
	vec2 uv;

	ScreenVertex() = default;
	ScreenVertex(vec2 pos, vec2 uv) : pos(pos), uv(uv) {}

	static void setupLayout();
};

class App {
public:
	App(IWindow* window);

	App(const App&) = delete;
	App& operator=(const App&) = delete;

	void addScreen(std::unique_ptr<Screen> screen) { screens.push_back(std::move(screen)); }

	void processEvent(const Event& event);

	void tick(microseconds dt, int windowWidth, int windowHeight) const;

private:
	IWindow* window;

	std::vector<std::unique_ptr<Screen>> screens;

	std::unique_ptr<Mesh<ScreenVertex>> quadMesh;
	std::vector<ScreenVertex> quadVertices;
	std::vector<Index> quadIndices;
};

#endif // APP_H
