#ifndef MAIN_H
#define MAIN_H

#ifdef WINDOWS_VERSION
#define NOMINMAX
#include <windows.h>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES

#else

#include <jni.h>
#include <android/asset_manager.h>
#include <android/imagedecoder.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#endif

typedef unsigned char byte;

#include <cmath>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "VectorMatrix.h"
#include "Utilities.h"

const float PI = 3.14159265359f;

#endif // MAIN_H
