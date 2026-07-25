#include "AndroidWindow.h"

void AndroidWindow::toggleFullscreen() {
	currentlyFullscreen = !currentlyFullscreen;

	// 1. Attach the C++ background thread to the JVM
	JNIEnv* env;
	app->activity->vm->AttachCurrentThread(&env, nullptr);

	// 2. Safely grab the Java object representing your GameActivity
	jobject activityObj = app->activity->javaGameActivity;
	jclass activityClass = env->GetObjectClass(activityObj);

	// 3. Look for our custom Java method: "void setImmersiveMode(boolean)"
	jmethodID methodId = env->GetMethodID(activityClass, "setImmersiveMode", "(Z)V");

	if (methodId) {
		// 4. Execute the Java method, passing the new state
		env->CallVoidMethod(activityObj, methodId, currentlyFullscreen);
	}

	// 5. Cleanly detach the thread
	app->activity->vm->DetachCurrentThread();
}

void AndroidWindow::updateWindowConfiguration() {
    config.width = ANativeWindow_getWidth(app->window);
    config.height = ANativeWindow_getHeight(app->window);
    int densityDpi = AConfiguration_getDensity(app->config);
    if (densityDpi != 0 && densityDpi != ACONFIGURATION_DENSITY_NONE)
        config.dpiScale = (float)densityDpi / (float)ACONFIGURATION_DENSITY_MEDIUM;
}
