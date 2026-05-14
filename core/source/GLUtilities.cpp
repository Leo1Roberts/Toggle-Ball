#include "GLUtilities.h"

void CheckGLError() {
#ifdef WINDOWS_VERSION
	GLenum err = glGetError();

	if (err == GL_NO_ERROR)
		return;

	const char* string = "unknown";
	switch (err) {
	case GL_INVALID_ENUM:
		string = "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		string = "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		string = "GL_INVALID_OPERATION";
		break;
	case GL_OUT_OF_MEMORY:
		string = "GL_OUT_OF_MEMORY";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		string = "GL_INVALID_FRAMEBUFFER_OPERATION";
		break;
	case 0x0507: //GL_CONTEXT_LOST:
		string = "GL_CONTEXT_LOST";
		break;
	case 0x8031: //GL_TABLE_TOO_LARGE1:
		string = "GL_TABLE_TOO_LARGE1";
		break;
	}

	char buf[1024];
	sprintf_s(buf, "Error %x: %s\n", err, string);
	printf(buf);
#endif
}