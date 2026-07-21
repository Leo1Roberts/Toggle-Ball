#include "main.h"
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
float dpiScale = 1.f;

/*!
 * Handles commands sent to this Android application
 * @param androidApp the app the commands are coming from
 * @param cmd the command to handle
 */
void handle_cmd(android_app *androidApp, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW: {
        if (androidApp->window == nullptr) break;

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
        ANativeWindow_setBuffersGeometry(androidApp->window, 0, 0, format);

        eglSurface = eglCreateWindowSurface(eglDisplay, config, androidApp->window, nullptr);

        if (eglContext == EGL_NO_CONTEXT) {
            EGLint contextAttributes[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE };
            eglContext = eglCreateContext(eglDisplay, config, nullptr, contextAttributes);
        }

        eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

        if (androidApp->userData == nullptr) {
            AndroidWindow window(androidApp);

            AssetManager::init(androidApp->activity->assetManager);
            auto app = std::make_unique<App>(&window);
            auto game = std::make_unique<Game>();
            game->play(LevelDescriptor::load("Level 1").get());
            app->addScreen(std::move(game));

            androidApp->userData = app.release();
        }
    } break;
    case APP_CMD_TERM_WINDOW: {
        // The window is hidden/destroyed. ONLY destroy the surface, NOT the context or App!
        if (eglDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (eglSurface != EGL_NO_SURFACE) {
                eglDestroySurface(eglDisplay, eglSurface);
                eglSurface = EGL_NO_SURFACE;
            }
        }
    } break;
    case APP_CMD_CONFIG_CHANGED: {
        int densityDpi = AConfiguration_getDensity(androidApp->config);
        dpiScale = (float)densityDpi / 160.0f;
    } break;
    case APP_CMD_DESTROY: {
        // The entire process is shutting down. Clean up all persistent state.
        if (androidApp->userData) {
            delete reinterpret_cast<App *>(androidApp->userData);
            androidApp->userData = nullptr;
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
    } break;
	default:
		break;
	}
}


void processInputEvents(struct android_app* androidApp, App* app) {
    // Swap and retrieve the input buffer for the current frame
    android_input_buffer* inputBuffer = android_app_swap_input_buffers(androidApp);
    if (!inputBuffer) return;

    if (inputBuffer->motionEventsCount > 0) {
        for (uint64_t i = 0; i < inputBuffer->motionEventsCount; ++i) {
            GameActivityMotionEvent* event = &inputBuffer->motionEvents[i];

            int32_t action = event->action;
            int32_t actionCode = action & AMOTION_EVENT_ACTION_MASK;
            size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                    >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            PointerButton button = PointerButton::Primary;

            switch (actionCode) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN: {
                auto* pointer = &event->pointers[pointerIndex];
                int32_t pointerId = pointer->id;
                float x = GameActivityPointerAxes_getX(pointer);
                float y = GameActivityPointerAxes_getY(pointer);

                app->processEvent(PointerEvent(pointerId, {x, y}, PointerAction::Down, button));
                break;
            }

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
            case AMOTION_EVENT_ACTION_CANCEL: {
                auto* pointer = &event->pointers[pointerIndex];
                int32_t pointerId = pointer->id;
                float x = GameActivityPointerAxes_getX(pointer);
                float y = GameActivityPointerAxes_getY(pointer);

                app->processEvent(PointerEvent(pointerId, {x, y}, PointerAction::Up, button));
                break;
            }

            case AMOTION_EVENT_ACTION_MOVE: {
                // AMOTION_EVENT_ACTION_MOVE contains coordinates for ALL active pointers
                for (uint32_t p = 0; p < event->pointerCount; ++p) {
                    auto* pointer = &event->pointers[p];
                    int32_t pointerId = pointer->id;
                    float x = GameActivityPointerAxes_getX(pointer);
                    float y = GameActivityPointerAxes_getY(pointer);

                    app->processEvent(PointerEvent(pointerId, {x, y}, PointerAction::Move, PointerButton::Unknown));
                }
                break;
            }

            default:
                break;
            }
        }

        // Clear motion events buffer after processing
        android_app_clear_motion_events(inputBuffer);
    }
}


/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *androidApp) {
	// Register an event handler for Android events
	androidApp->onAppCmd = handle_cmd;

    int densityDpi = AConfiguration_getDensity(androidApp->config);
    if (densityDpi != 0 && densityDpi != ACONFIGURATION_DENSITY_NONE)
        dpiScale = (float)densityDpi / (float)ACONFIGURATION_DENSITY_MEDIUM;

    android_app_set_key_event_filter(androidApp, nullptr);
	android_app_set_motion_event_filter(androidApp, nullptr);

    microseconds t1 = now();

	// This sets up a typical game/event loop. It will run until the app is destroyed.
	do {
		// Process all pending events before running game logic.
		bool done = false;
		while (!done) {
			int events;
			android_poll_source *pSource;
			int result = ALooper_pollOnce(0, nullptr, &events, reinterpret_cast<void **>(&pSource));

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
				if (pSource)
					pSource->process(androidApp, pSource);
			}
		}

        if (androidApp->userData && eglSurface != EGL_NO_SURFACE) {
            auto *app = reinterpret_cast<App *>(androidApp->userData);

            processInputEvents(androidApp, app);

            eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &width);
            eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &height);

            microseconds t2 = now();
            app->tick(t2 - t1, width, height, dpiScale);
            t1 = t2;

            if (eglSwapBuffers(eglDisplay, eglSurface) == EGL_FALSE) {
                EGLint err = eglGetError();
                if (err == EGL_BAD_SURFACE || err == EGL_CONTEXT_LOST) {
                    // Handle lost surface/context gracefully if OS forced teardown
                }
            }
        }
	} while (!androidApp->destroyRequested);
}
}