#pragma once

#include "glfw-common.hpp"


#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif __linux__
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_GLX
#define GLFW_EXPOSE_NATIVE_EGL
#endif
#include <GLFW/glfw3native.h>


namespace glfw {
DBG_EXPORT JS_METHOD(platformDevice);
DBG_EXPORT JS_METHOD(platformWindow);
DBG_EXPORT JS_METHOD(platformContext);
DBG_EXPORT JS_METHOD(getPlatform);
DBG_EXPORT JS_METHOD(platformSupported);
} // namespace glfw
