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