#include "main.h"

#include "App.h"
#include "Game.h"
#include "Game_OLD.h"
#include "GlfwWindow.h"

#include <iostream>

inline unsigned max_unsigned(unsigned a, unsigned b) { return (a > b) ? a : b; }

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}


[[nodiscard]] static KeyCode translateKey(int glfwKey) {
	switch (glfwKey) {
	case GLFW_KEY_SPACE:	return KeyCode::Space;
	case GLFW_KEY_F11:		return KeyCode::F11;
	default:				return KeyCode::Unknown;
	}
}

[[nodiscard]] static KeyAction translateKeyAction(int glfwKeyAction) {
	switch (glfwKeyAction) {
	case GLFW_PRESS:	return KeyAction::Down;
	case GLFW_REPEAT:	return KeyAction::Repeat;
	case GLFW_RELEASE:	return KeyAction::Up;
	default:			return KeyAction::Unknown;
	}
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->processEvent(KeyEvent(translateKey(key), translateKeyAction(action)));
}

int main() {
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_DEBUG, GL_TRUE);
	// glfwWindowHint(GLFW_SAMPLES, 4);

	int width = 1600, height = 1000;

	GLFWwindow* rawWindow = glfwCreateWindow(width, height, "Toggle Ball", nullptr, nullptr);

	if (!rawWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(rawWindow);

	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // For GLAD 2 use the following instead: gladLoadGL(glfwGetProcAddress)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	// glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
	//
	// Game::createGame(window);
	// glfwSetKeyCallback(window, &Game::handleKeyInput);
	// glfwSetCharCallback(window, &Game::handleCharInput);
	// glfwSetCursorPosCallback(window, &Game::handleCursorPosInput);
	// glfwSetCursorEnterCallback(window, &Game::handleCursorEnterEvent);
	// glfwSetMouseButtonCallback(window, &Game::handleMouseButtonInput);
	// glfwSetScrollCallback(window, &Game::handleScrollInput);

	glfwSetKeyCallback(rawWindow, keyCallback);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	GlfwWindow window(rawWindow);

	App app(&window);
	std::unique_ptr<Game> game = std::make_unique<Game>(width, height);
	game->play(LevelDescriptor::load("Level 1").get());
	app.addScreen(std::move(game));

	glfwSetWindowUserPointer(rawWindow, &app);

	microseconds t1 = now();

	do {
		glfwPollEvents();

		glfwGetFramebufferSize(rawWindow, &width, &height);
		microseconds t2 = now();
		app.tick(t2 - t1, width, height);
		t1 = t2;
		glfwSwapBuffers(rawWindow);
	} while (!glfwWindowShouldClose(rawWindow));
}
