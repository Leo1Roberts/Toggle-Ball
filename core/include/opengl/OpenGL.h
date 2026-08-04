#ifndef OPENGL_H
#define OPENGL_H


#if defined(PLATFORM_DESKTOP)
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
#elif defined(PLATFORM_ANDROID)
	#include <GLES3/gl3.h>
	#include <EGL/egl.h>
#endif


#endif // OPENGL_H
