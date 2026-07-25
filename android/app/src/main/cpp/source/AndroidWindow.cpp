#include "AndroidWindow.h"

void AndroidWindow::toggleFullscreen() {
	currentlyFullscreen = !currentlyFullscreen;

	JNIEnv* env;
	app->activity->vm->AttachCurrentThread(&env, nullptr);

	jobject activityObj = app->activity->javaGameActivity;
	jclass activityClass = env->GetObjectClass(activityObj);

	jmethodID methodId = env->GetMethodID(activityClass, "setImmersiveMode", "(Z)V");

	if (methodId)
		env->CallVoidMethod(activityObj, methodId, currentlyFullscreen);

	app->activity->vm->DetachCurrentThread();
}
