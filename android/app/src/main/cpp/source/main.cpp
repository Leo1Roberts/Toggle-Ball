#include "AndroidWindow.h"
#include "App.h"
#include "utilities/AssetManager.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "game/GameMode.h"
#include "editor/EditorMode.h"

#include <game-activity/GameActivity.cpp>
#include <game-activity/native_app_glue/android_native_app_glue.c>
#include <game-text-input/gametextinput.cpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

extern "C" {

EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;


void handle_cmd(android_app *androidApp, int cmd) {
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
            AssetManager::init(androidApp->activity->assetManager);
            Settings::load();
            Meshes::load();
            Shaders::load();
            Textures::load();
            Fonts::load();

            androidApp->userData = new App(std::make_unique<AndroidWindow>(androidApp), std::make_unique<GameMode>());
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
    case APP_CMD_WINDOW_RESIZED: {
        if (androidApp->userData) {
            auto *app = reinterpret_cast<App *>(androidApp->userData);
            app->resizeWindow();
        }
    }
    case APP_CMD_CONFIG_CHANGED: {
        if (androidApp->userData) {
            auto *app = reinterpret_cast<App *>(androidApp->userData);
            app->updateDPIScale();
        }
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

glm::vec2 pointer0Position;
constexpr float dragThreshold = 8.f;
bool dragging = false;
glm::vec2 pointer0DownPosition;

void processInputEvents(struct android_app* androidApp, App* app) {
    // Swap and retrieve the input buffer for the current frame
    android_input_buffer* inputBuffer = android_app_swap_input_buffers(androidApp);
    if (!inputBuffer) return;

    if (inputBuffer->motionEventsCount > 0) {
        for (int i = 0; i < inputBuffer->motionEventsCount; ++i) {
            GameActivityMotionEvent* event = &inputBuffer->motionEvents[i];

            int action = event->action;
            int actionCode = action & AMOTION_EVENT_ACTION_MASK;
            int pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                    >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            PointerButton button = PointerButton::Primary;

            switch (actionCode) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN: {
                auto* pointer = &event->pointers[pointerIndex];
                int pointerId = pointer->id;
                float x = GameActivityPointerAxes_getX(pointer);
                float y = GameActivityPointerAxes_getY(pointer);

                if (pointerId == 0) {
                    pointer0Position = pointer0DownPosition = {x, y};
                    pointer0DownPosition = {x, y};
                    dragging = false;
                }

                app->processEvent(PointerEvent(pointerId, {x, y}, PointerAction::Down, button));
                break;
            }

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
            case AMOTION_EVENT_ACTION_CANCEL: {
                auto* pointer = &event->pointers[pointerIndex];
                int pointerId = pointer->id;
                float x = GameActivityPointerAxes_getX(pointer);
                float y = GameActivityPointerAxes_getY(pointer);

                app->processEvent(PointerEvent(pointerId, {x, y}, PointerAction::Up, button));

                if (pointerId == 0 && dragging) {
                    dragging = false;
                    app->processEvent(PointerEvent(pointerId, {x, y}, PointerAction::FinishDrag));
                }

                app->processEvent(PointerEvent(pointerId, {}, PointerAction::Leave));

                break;
            }

            case AMOTION_EVENT_ACTION_MOVE: {
                // AMOTION_EVENT_ACTION_MOVE contains coordinates for ALL active pointers
                for (int p = 0; p < event->pointerCount; ++p) {
                    auto* pointer = &event->pointers[p];
                    int pointerId = pointer->id;
                    float x = GameActivityPointerAxes_getX(pointer);
                    float y = GameActivityPointerAxes_getY(pointer);

                    if (pointerId == 0) {
                        if (!dragging && length2(glm::vec2(x, y) - pointer0DownPosition) > dragThreshold * dragThreshold) {
                            dragging = true;
                            app->processEvent(PointerEvent(pointerId, pointer0Position, PointerAction::StartDrag, button));
                        }

                        pointer0Position = {x, y};
                    }

                    app->processEvent(PointerEvent(pointerId, {x, y}, dragging ? PointerAction::Drag : PointerAction::Move));
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

            microseconds t2 = now();
            app->tick(t2 - t1);
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