#include "main.h"

#include "App.h"
#include "EditorMode.h"
#include "GameMode.h"
#include "PlayScreen.h"
#include "GlfwWindow.h"
#include "Shader.h"

#include <cfenv>
#include <iostream>

inline unsigned max_unsigned(unsigned a, unsigned b) { return (a > b) ? a : b; }

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source) {
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	default:;
	} std::cout << std::endl;

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	default:;
	} std::cout << std::endl;

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	default:;
	} std::cout << std::endl;

	std::cout << std::endl;
}


[[nodiscard]] static byte translateMods(int glfwMods) {
	byte mods = MOD_NONE;

	if (glfwMods & GLFW_MOD_CONTROL) mods |= MOD_CTRL;
	if (glfwMods & GLFW_MOD_SHIFT)   mods |= MOD_SHIFT;
	if (glfwMods & GLFW_MOD_ALT)     mods |= MOD_ALT;

	return mods;
}

[[nodiscard]] static KeyCode translateKey(int glfwKey) {
	switch (glfwKey) {
	case GLFW_KEY_TAB:		return KeyCode::Tab;
	case GLFW_KEY_ENTER:	return KeyCode::Enter;
	case GLFW_KEY_ESCAPE:	return KeyCode::Escape;
	case GLFW_KEY_SPACE:	return KeyCode::Space;
	case GLFW_KEY_Z:		return KeyCode::Z;
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
	app->processEvent(KeyEvent({translateKey(key), translateMods(mods)}, translateKeyAction(action)));
}


[[nodiscard]] static PointerButton translateMouseButton(int glfwMouseButton) {
	switch (glfwMouseButton) {
	case GLFW_MOUSE_BUTTON_LEFT:	return PointerButton::Primary;
	case GLFW_MOUSE_BUTTON_RIGHT:	return PointerButton::Secondary;
	case GLFW_MOUSE_BUTTON_MIDDLE:	return PointerButton::Tertiary;
	default:						return PointerButton::Unknown;
	}
}

[[nodiscard]] static PointerAction translateMouseButtonAction(int glfwMouseButtonAction) {
	switch (glfwMouseButtonAction) {
	case GLFW_PRESS:	return PointerAction::Down;
	case GLFW_RELEASE:	return PointerAction::Up;
	default:			return PointerAction::Unknown;
	}
}

glm::vec2 mousePosition;

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->processEvent(PointerEvent(0, mousePosition, translateMouseButtonAction(action), translateMouseButton(button)));
}

static void cursorPosCallback(GLFWwindow* window, double x, double y) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
#if defined(PLATFORM_LINUX)
	mousePosition = glm::vec2(x, y) * app->window->config.dpiScale;
#else
	mousePosition = {x, y};
#endif
	app->processEvent(PointerEvent(0, mousePosition, PointerAction::Move, PointerButton::Unknown));
}

static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->processEvent(PointerEvent(0, mousePosition, PointerAction::Scroll, PointerButton::Unknown, {xOffset, yOffset}));
}


static void framebufferSizeCallback(GLFWwindow* window, int, int) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->resizeWindow();
}

static void windowContentScaleCallback(GLFWwindow* window, float, float) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->updateDPIScale();
}

int main() {
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_DEBUG, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	glfwSetKeyCallback(rawWindow, keyCallback);
	glfwSetMouseButtonCallback(rawWindow, mouseButtonCallback);
	glfwSetCursorPosCallback(rawWindow, cursorPosCallback);
	glfwSetScrollCallback(rawWindow, scrollCallback);

	glfwSetFramebufferSizeCallback(rawWindow, framebufferSizeCallback);
	glfwSetWindowContentScaleCallback(rawWindow, windowContentScaleCallback);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	Settings::load();
	Meshes::load();
	Shaders::load();
	Textures::load();
	Fonts::load();

	App app(std::make_unique<GlfwWindow>(rawWindow), std::make_unique<EditorMode>());

	glfwSetWindowUserPointer(rawWindow, &app);

	microseconds t1 = now();

	do {
		glfwPollEvents();

		microseconds t2 = now();
		app.tick(t2 - t1);
		t1 = t2;
		glfwSwapBuffers(rawWindow);
	} while (!glfwWindowShouldClose(rawWindow));
}
