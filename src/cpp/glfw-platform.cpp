#include "glfw-common.hpp"
#include "glfw-platform.hpp"

#ifdef __APPLE__
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLDevice.h>
#endif


namespace glfw {

DBG_EXPORT JS_METHOD(platformDevice) {
	NAPI_ENV;
#ifdef _WIN32
	RET_PTR(wglGetCurrentDC());
#elif __linux__
	if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
		RET_PTR(glfwGetWaylandDisplay());
	} else {
		RET_PTR(glfwGetX11Display());
	}
#elif __APPLE__
	CGLContextObj kCGLContext = CGLGetCurrentContext();
	RET_PTR(CGLGetShareGroup(kCGLContext));
#endif
}


DBG_EXPORT JS_METHOD(platformWindow) {
	NAPI_ENV;
	THIS_WINDOW;
#ifdef _WIN32
	RET_PTR(glfwGetWin32Window(window));
#elif __linux__
	if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
		RET_PTR(glfwGetWaylandWindow(window));
	} else {
		RET_PTR(static_cast<int64_t>(glfwGetX11Window(window)));
	}
#elif __APPLE__
	RET_PTR(glfwGetCocoaWindow(window));
#endif
}


DBG_EXPORT JS_METHOD(platformContext) {
	NAPI_ENV;
	THIS_WINDOW;
#ifdef _WIN32
	RET_PTR(glfwGetWGLContext(window));
#elif __linux__
	if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
		RET_PTR(glfwGetEGLContext(window));
	} else {
		RET_PTR(glfwGetGLXContext(window));
	}
#elif __APPLE__
	RET_PTR(glfwGetNSGLContext(window));
#endif
}


DBG_EXPORT JS_METHOD(getPlatform) {
	NAPI_ENV;
	RET_NUM(glfwGetPlatform());
}

DBG_EXPORT JS_METHOD(platformSupported) {
	NAPI_ENV;
	REQ_INT_ARG(0, platform);
	RET_NUM(glfwPlatformSupported(platform));
}

} // namespace glfw
