#include "main.h"
#include "Game_OLD.h"

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>
#include <App.h>
#include <Game.h>

extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;

EGLint width, height;

/*!
 * Handles commands sent to this Android application
 * @param pApp the app the commands are coming from
 * @param cmd the command to handle
 */
void handle_cmd(android_app *pApp, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW: {
		// A new window is created, associate a renderer with it. You may replace this with a
		// "game" class if that suits your needs. Remember to change all instances of userData
		// if you change the class here as a reinterpret_cast is dangerous this in the
		// android_main function and the APP_CMD_TERM_WINDOW handler case.

//		Game_OLD::createGame(pApp);
//		pApp->userData = (void *) true;

		constexpr EGLint attribs[] = {
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_BLUE_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_RED_SIZE, 8,
				EGL_DEPTH_SIZE, 24,
				EGL_NONE
		};

		// The default display is probably what you want on Android
		auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		eglInitialize(display, nullptr, nullptr);

		// figure out how many configs there are
		EGLint numConfigs;
		eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

		if (numConfigs == 0) {
			return;
		}

		// get the list of configurations
		std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
		eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

		// Find a config we like.
		// Could likely just grab the first if we don't care about anything else in the config.
		// Otherwise hook in your own heuristic
		EGLConfig config = nullptr;
		for (int i = 0; i < numConfigs; i++) {
			EGLConfig c = supportedConfigs[i];
			EGLint red, green, blue, depth;
			if (eglGetConfigAttrib(display, c, EGL_RED_SIZE, &red) &&
				eglGetConfigAttrib(display, c, EGL_GREEN_SIZE, &green) &&
				eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &blue) &&
				eglGetConfigAttrib(display, c, EGL_DEPTH_SIZE, &depth)) {
				if (red == 8 && green == 8 && blue == 8 && depth == 24) {
					config = c;
					break;
				}
			}
		}

		if (!config) {
			config = supportedConfigs[0];
		}

		// create the proper window surface
		EGLint format;
		eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
		ANativeWindow_setBuffersGeometry(pApp->window, 0, 0, format);
		EGLSurface surface = eglCreateWindowSurface(display, config, pApp->window, nullptr);

		// Create a GLES 3 context
		EGLint contextAttribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE };
		EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

		// get some window metrics
		eglMakeCurrent(display, surface, surface, context);

		eglDisplay = display;
		eglSurface = surface;
		eglContext = context;

        eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);
        eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);

		AssetManager::init(pApp->activity->assetManager);
		std::unique_ptr<App> app = std::make_unique<App>();
		std::unique_ptr<Game> game = std::make_unique<Game>(width, height);
		game->play(LevelDescriptor::load("Dev test").get());
		app->addScreen(std::move(game));
		pApp->userData = app.release();
	}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being destroyed. Use this to clean up your userData to avoid leaking
		// resources.
		//
		// We have to check if userData is assigned just in case this comes in really quickly

//		if (pApp->userData) Game_OLD::deleteGame();
		pApp->userData = nullptr;

		if (eglDisplay != EGL_NO_DISPLAY) {
			eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			if (eglContext != EGL_NO_CONTEXT) {
				eglDestroyContext(eglDisplay, eglContext);
				eglContext = EGL_NO_CONTEXT;
			}
			if (eglSurface != EGL_NO_SURFACE) {
				eglDestroySurface(eglDisplay, eglSurface);
				eglSurface = EGL_NO_SURFACE;
			}
			eglTerminate(eglDisplay);
			eglDisplay = EGL_NO_DISPLAY;
		}
		break;
	default:
		break;
	}
}

/*!
 * Enable the motion events you want to handle; not handled events are
 * passed back to OS for further processing. For this example case,
 * only pointer and joystick devices are enabled.
 *
 * @param motionEvent the newly arrived GameActivityMotionEvent.
 * @return true if the event is from a pointer or joystick device,
 *		 false for all other input devices.
 */
bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
	auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
	return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
			sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *pApp) {
	// Register an event handler for Android events
	pApp->onAppCmd = handle_cmd;

	// Set input event filters (set it to NULL if the app wants to process all inputs).
	// Note that for key inputs, this example uses the default default_key_filter()
	// implemented in android_native_app_glue.c.
	android_app_set_motion_event_filter(pApp, motion_event_filter_func);
	float t1 = float(now_ms()) / 1000.0f;

	// This sets up a typical game/event loop. It will run until the app is destroyed.
	do {
		// Process all pending events before running game logic.
		bool done = false;
		while (!done) {
			// 0 is non-blocking.
			int timeout = 0;
			int events;
			android_poll_source *pSource;
			int result = ALooper_pollOnce(timeout, nullptr, &events,
										  reinterpret_cast<void **>(&pSource));
			switch (result) {
			case ALOOPER_POLL_TIMEOUT:
				[[clang::fallthrough]];
			case ALOOPER_POLL_WAKE:
				// No events occurred before the timeout or explicit wake. Stop checking for events.
				done = true;
				break;
			case ALOOPER_EVENT_ERROR:
			case ALOOPER_POLL_CALLBACK:
				break;
			default:
				if (pSource) {
					pSource->process(pApp, pSource);
				}
			}
		}

		// Check if any user data is associated. This is assigned in handle_cmd
		if (pApp->userData) {
//			// We know that our user data is a Game, so reinterpret cast it. If you change your
//			// user data remember to change it here
//			auto *pGame = reinterpret_cast<Game_OLD *>(pApp->userData);
//
//			// Process game input
//			pGame->handleInput();
//
//			// Render a frame
//			pGame->update();

			auto *app = reinterpret_cast<App *>(pApp->userData);

			eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);
			eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);

			float t2 = float(now_ms()) / 1000.0f;
			app->tick(t2 - t1, width, height);
			t1 = t2;
			eglSwapBuffers(eglDisplay, eglSurface);
		}
	} while (!pApp->destroyRequested);
}
}