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


[[nodiscard]] static byte getUpdatedMods(GLFWwindow* window) {
	byte mods = MOD_NONE;

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		mods |= MOD_CTRL;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		mods |= MOD_SHIFT;

	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
		mods |= MOD_ALT;

	return mods;
}

[[nodiscard]] static KeyCode translateKey(int glfwKey) {
	switch (glfwKey) {
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_RIGHT_CONTROL: return KeyCode::Ctrl;
	case GLFW_KEY_LEFT_SHIFT:
	case GLFW_KEY_RIGHT_SHIFT:   return KeyCode::Shift;
	case GLFW_KEY_LEFT_ALT:
	case GLFW_KEY_RIGHT_ALT:     return KeyCode::Alt;

	case GLFW_KEY_TAB:       return KeyCode::Tab;
	case GLFW_KEY_ENTER:     return KeyCode::Enter;
	case GLFW_KEY_ESCAPE:    return KeyCode::Escape;
	case GLFW_KEY_SPACE:     return KeyCode::Space;
	case GLFW_KEY_BACKSPACE: return KeyCode::Backspace;

	case GLFW_KEY_A: return KeyCode::A;
	case GLFW_KEY_B: return KeyCode::B;
	case GLFW_KEY_C: return KeyCode::C;
	case GLFW_KEY_D: return KeyCode::D;
	case GLFW_KEY_E: return KeyCode::E;
	case GLFW_KEY_F: return KeyCode::F;
	case GLFW_KEY_G: return KeyCode::G;
	case GLFW_KEY_H: return KeyCode::H;
	case GLFW_KEY_I: return KeyCode::I;
	case GLFW_KEY_J: return KeyCode::J;
	case GLFW_KEY_K: return KeyCode::K;
	case GLFW_KEY_L: return KeyCode::L;
	case GLFW_KEY_M: return KeyCode::M;
	case GLFW_KEY_N: return KeyCode::N;
	case GLFW_KEY_O: return KeyCode::O;
	case GLFW_KEY_P: return KeyCode::P;
	case GLFW_KEY_Q: return KeyCode::Q;
	case GLFW_KEY_R: return KeyCode::R;
	case GLFW_KEY_S: return KeyCode::S;
	case GLFW_KEY_T: return KeyCode::T;
	case GLFW_KEY_U: return KeyCode::U;
	case GLFW_KEY_V: return KeyCode::V;
	case GLFW_KEY_W: return KeyCode::W;
	case GLFW_KEY_X: return KeyCode::X;
	case GLFW_KEY_Y: return KeyCode::Y;
	case GLFW_KEY_Z: return KeyCode::Z;

	case GLFW_KEY_0: return KeyCode::Num0;
	case GLFW_KEY_1: return KeyCode::Num1;
	case GLFW_KEY_2: return KeyCode::Num2;
	case GLFW_KEY_3: return KeyCode::Num3;
	case GLFW_KEY_4: return KeyCode::Num4;
	case GLFW_KEY_5: return KeyCode::Num5;
	case GLFW_KEY_6: return KeyCode::Num6;
	case GLFW_KEY_7: return KeyCode::Num7;
	case GLFW_KEY_8: return KeyCode::Num8;
	case GLFW_KEY_9: return KeyCode::Num9;

	case GLFW_KEY_KP_0: return KeyCode::Numpad0;
	case GLFW_KEY_KP_1: return KeyCode::Numpad1;
	case GLFW_KEY_KP_2: return KeyCode::Numpad2;
	case GLFW_KEY_KP_3: return KeyCode::Numpad3;
	case GLFW_KEY_KP_4: return KeyCode::Numpad4;
	case GLFW_KEY_KP_5: return KeyCode::Numpad5;
	case GLFW_KEY_KP_6: return KeyCode::Numpad6;
	case GLFW_KEY_KP_7: return KeyCode::Numpad7;
	case GLFW_KEY_KP_8: return KeyCode::Numpad8;
	case GLFW_KEY_KP_9: return KeyCode::Numpad9;

	case GLFW_KEY_F1:  return KeyCode::F1;
	case GLFW_KEY_F2:  return KeyCode::F2;
	case GLFW_KEY_F3:  return KeyCode::F3;
	case GLFW_KEY_F4:  return KeyCode::F4;
	case GLFW_KEY_F5:  return KeyCode::F5;
	case GLFW_KEY_F6:  return KeyCode::F6;
	case GLFW_KEY_F7:  return KeyCode::F7;
	case GLFW_KEY_F8:  return KeyCode::F8;
	case GLFW_KEY_F9:  return KeyCode::F9;
	case GLFW_KEY_F10: return KeyCode::F10;
	case GLFW_KEY_F11: return KeyCode::F11;
	case GLFW_KEY_F12: return KeyCode::F12;
	case GLFW_KEY_F13: return KeyCode::F13;
	case GLFW_KEY_F14: return KeyCode::F14;
	case GLFW_KEY_F15: return KeyCode::F15;
	case GLFW_KEY_F16: return KeyCode::F16;
	case GLFW_KEY_F17: return KeyCode::F17;
	case GLFW_KEY_F18: return KeyCode::F18;
	case GLFW_KEY_F19: return KeyCode::F19;
	case GLFW_KEY_F20: return KeyCode::F20;
	case GLFW_KEY_F21: return KeyCode::F21;
	case GLFW_KEY_F22: return KeyCode::F22;
	case GLFW_KEY_F23: return KeyCode::F23;
	case GLFW_KEY_F24: return KeyCode::F24;
	case GLFW_KEY_F25: return KeyCode::F25;

	case GLFW_KEY_UP:        return KeyCode::Up;
	case GLFW_KEY_DOWN:      return KeyCode::Down;
	case GLFW_KEY_LEFT:      return KeyCode::Left;
	case GLFW_KEY_RIGHT:     return KeyCode::Right;
	case GLFW_KEY_HOME:      return KeyCode::Home;
	case GLFW_KEY_END:       return KeyCode::End;
	case GLFW_KEY_PAGE_UP:   return KeyCode::PageUp;
	case GLFW_KEY_PAGE_DOWN: return KeyCode::PageDown;

	default: return KeyCode::Unknown;
	}
}

[[nodiscard]] static KeyAction translateKeyAction(int glfwKeyAction) {
	switch (glfwKeyAction) {
	case GLFW_PRESS:   return KeyAction::Down;
	case GLFW_REPEAT:  return KeyAction::Repeat;
	case GLFW_RELEASE: return KeyAction::Up;
	default:           return KeyAction::Unknown;
	}
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->processEvent(KeyEvent({translateKey(key), getUpdatedMods(window)}, translateKeyAction(action)));
}


static void charCallback(GLFWwindow* window, unsigned int codepoint) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	if (codepoint <= CHAR_MAX) // Ignore fancy Unicode characters
		app->processEvent((char)codepoint);
}


[[nodiscard]] static PointerButton translateMouseButton(int glfwMouseButton) {
	switch (glfwMouseButton) {
	case GLFW_MOUSE_BUTTON_LEFT:   return PointerButton::Primary;
	case GLFW_MOUSE_BUTTON_RIGHT:  return PointerButton::Secondary;
	case GLFW_MOUSE_BUTTON_MIDDLE: return PointerButton::Tertiary;
	default:                       return PointerButton::Unknown;
	}
}

[[nodiscard]] static PointerAction translateMouseButtonAction(int glfwMouseButtonAction) {
	switch (glfwMouseButtonAction) {
	case GLFW_PRESS:   return PointerAction::Down;
	case GLFW_RELEASE: return PointerAction::Up;
	default:           return PointerAction::Unknown;
	}
}

glm::vec2 mousePosition;

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	auto* app = (App*)glfwGetWindowUserPointer(window);
	if (!app) return;
	app->processEvent(PointerEvent(0, mousePosition, translateMouseButtonAction(action), translateMouseButton(button), getUpdatedMods(window)));
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
	app->processEvent(PointerEvent(0, mousePosition, PointerAction::Scroll, PointerButton::Unknown, 0, {xOffset, yOffset}));
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
#if defined(PLATFORM_LINUX)
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

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
	glfwSetCharCallback(rawWindow, charCallback);
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
