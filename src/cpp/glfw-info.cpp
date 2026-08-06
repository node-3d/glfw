#include "glfw-common.hpp"
#include "glfw-info.hpp"
#include "glfw-window.hpp"


namespace glfw {

constexpr int REQUEST_ADAPTIVE_SWAP_INTERVAL = -1;
constexpr int REQUEST_DWM_FLUSH_SWAP_INTERVAL = -2;

bool isNegativeSwapIntervalSupported() {
	return glfwExtensionSupported("WGL_EXT_swap_control_tear") == GLFW_TRUE ||
	    glfwExtensionSupported("GLX_EXT_swap_control_tear") == GLFW_TRUE;
}


int resolveAdaptiveSwapInterval() {
	return isNegativeSwapIntervalSupported() ? REQUEST_ADAPTIVE_SWAP_INTERVAL : 1;
}


int resolveSwapInterval(int interval) {
#ifdef _WIN32
	if (interval == REQUEST_DWM_FLUSH_SWAP_INTERVAL) {
		GLFWwindow *window = glfwGetCurrentContext();
		if (window && glfwGetWindowMonitor(window)) {
			return resolveAdaptiveSwapInterval();
		}

		return 0;
	}
#else
	if (interval == REQUEST_DWM_FLUSH_SWAP_INTERVAL) {
		return resolveAdaptiveSwapInterval();
	}
#endif

	if (interval == REQUEST_ADAPTIVE_SWAP_INTERVAL && !isNegativeSwapIntervalSupported()) {
		return 1;
	}

	return interval;
}


void rememberSwapInterval(int interval) {
	GLFWwindow *window = glfwGetCurrentContext();
	if (!window) {
		return;
	}

	auto *state = reinterpret_cast<WinState *>(glfwGetWindowUserPointer(window));
	if (!state) {
		return;
	}

	state->swapInterval = interval;
}

DBG_EXPORT JS_METHOD(getError) {
	NAPI_ENV;
	const char *err;
	int code = glfwGetError(&err);

	if (code != GLFW_NO_ERROR) {
		RET_STR(err);
	}

	RET_NULL;
}


DBG_EXPORT JS_METHOD(getFramebufferSize) {
	NAPI_ENV;
	THIS_WINDOW;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	Napi::Object obj = JS_OBJECT;
	obj.Set("width", JS_NUM(width));
	obj.Set("height", JS_NUM(height));

	RET_VALUE(obj);
}


DBG_EXPORT JS_METHOD(swapBuffers) {
	NAPI_ENV;
	THIS_WINDOW;
	glfwSwapBuffers(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(swapInterval) {
	NAPI_ENV;
	REQ_INT32_ARG(0, interval);

	rememberSwapInterval(interval);
	glfwSwapInterval(resolveSwapInterval(interval));
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(extensionSupported) {
	NAPI_ENV;
	REQ_STR_ARG(0, str);

	RET_BOOL(glfwExtensionSupported(str.c_str()) == GLFW_TRUE);
}

} // namespace glfw
