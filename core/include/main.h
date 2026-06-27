#ifndef MAIN_H
#define MAIN_H

#include <string>

#if defined(PLATFORM_DESKTOP)
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define USE_MATH_DEFINES

#include <cstring>
#include <climits>

#elif defined(PLATFORM_ANDROID)

#include <jni.h>
#include <android/asset_manager.h>
#include <android/imagedecoder.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#endif

using byte = unsigned char;

#include <stdexcept>
#include <cmath>
#include <memory>
#include <array>
#include <vector>
#include <map>
#include "VectorMatrix.h"
#include "GLUtilities.h"
#include "Utilities.h"

constexpr float PI = 3.14159265359f;
constexpr float PHYSICS_TIMESTEP = 0.001f;

#endif // MAIN_H