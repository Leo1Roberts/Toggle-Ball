#include "main.h"
#include "Game_OLD.h"
#include "AndroidWindow.h"

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
        if (pApp->window == nullptr) break;

        // 1. Initialize Display & Context ONLY ONCE on cold start
        if (eglDisplay == EGL_NO_DISPLAY) {
            eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            eglInitialize(eglDisplay, nullptr, nullptr);
        }

        constexpr EGLint attributes[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_NONE
        };

        EGLint numConfigs;
        eglChooseConfig(eglDisplay, attributes, nullptr, 0, &numConfigs);
        std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
        eglChooseConfig(eglDisplay, attributes, supportedConfigs.get(), numConfigs, &numConfigs);

        EGLConfig config = supportedConfigs[0];

        EGLint format;
        eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
        ANativeWindow_setBuffersGeometry(pApp->window, 0, 0, format);

        eglSurface = eglCreateWindowSurface(eglDisplay, config, pApp->window, nullptr);

        if (eglContext == EGL_NO_CONTEXT) {
            EGLint contextAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE };
            eglContext = eglCreateContext(eglDisplay, config, nullptr, contextAttributes);
        }

        eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

        if (pApp->userData == nullptr) {
            eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);
            eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);

            AndroidWindow window(pApp);

            AssetManager::init(pApp->activity->assetManager);
            auto app = std::make_unique<App>(&window);
            auto game = std::make_unique<Game>(width, height);
            game->play(LevelDescriptor::load("Level 1").get());
            app->addScreen(std::move(game));

            pApp->userData = app.release();
        }
        break;
    }
    case APP_CMD_TERM_WINDOW: {
        // The window is hidden/destroyed. ONLY destroy the surface, NOT the context or App!
        if (eglDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (eglSurface != EGL_NO_SURFACE) {
                eglDestroySurface(eglDisplay, eglSurface);
                eglSurface = EGL_NO_SURFACE;
            }
        }
        break;
    }
    case APP_CMD_DESTROY: {
        // The entire process is shutting down. Clean up all persistent state.
        if (pApp->userData) {
            delete reinterpret_cast<App *>(pApp->userData);
            pApp->userData = nullptr;
        }

        if (eglDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (eglContext != EGL_NO_CONTEXT) {
                eglDestroyContext(eglDisplay, eglContext);
                eglContext = EGL_NO_CONTEXT;
            }
            eglTerminate(eglDisplay);
            eglDisplay = EGL_NO_DISPLAY;
        }
        break;
    }
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
    microseconds t1 = now();

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

        if (pApp->userData && eglSurface != EGL_NO_SURFACE) {
            auto *app = reinterpret_cast<App *>(pApp->userData);

            eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);
            eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);

            microseconds t2 = now();
            app->tick(t2 - t1, width, height);
            t1 = t2;

            if (eglSwapBuffers(eglDisplay, eglSurface) == EGL_FALSE) {
                EGLint err = eglGetError();
                if (err == EGL_BAD_SURFACE || err == EGL_CONTEXT_LOST) {
                    // Handle lost surface/context gracefully if OS forced teardown
                }
            }
        }
	} while (!pApp->destroyRequested);
}
}