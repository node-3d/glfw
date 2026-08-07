#include "glfw-common.hpp"
#include "glfw-info.hpp"
#include "glfw-window.hpp"


namespace glfw {

constexpr int REQUEST_NEGATIVE_SWAP_INTERVAL = -1;
constexpr int REQUEST_VSYNC_SWAP_INTERVAL = 1;

int normalizeSwapInterval(int interval) {
	if (interval == 0) {
		return 0;
	}

	return interval < 0 ? REQUEST_NEGATIVE_SWAP_INTERVAL : REQUEST_VSYNC_SWAP_INTERVAL;
}

int resolveSwapInterval(WinState *state, int interval) {
	interval = normalizeSwapInterval(interval);

	if (!state) {
		return interval;
	}

	state->isSoftwarePaced = false;

	if (interval == 0) {
		return 0;
	}

	if (interval == REQUEST_NEGATIVE_SWAP_INTERVAL || interval == REQUEST_VSYNC_SWAP_INTERVAL) {
		state->isSoftwarePaced = true;

		if (interval == REQUEST_NEGATIVE_SWAP_INTERVAL) {
			return state->isNegativeSwapIntervalSupported ? REQUEST_NEGATIVE_SWAP_INTERVAL
			                                              : REQUEST_VSYNC_SWAP_INTERVAL;
		}

		return REQUEST_VSYNC_SWAP_INTERVAL;
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

	state->swapInterval = normalizeSwapInterval(interval);
	state->nextFrameStartedAt = std::chrono::steady_clock::time_point::min();
	state->frameTimestampAt = std::chrono::steady_clock::time_point::min();
	updateWindowRefreshRate(window);
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
	if (!shouldRenderFrame(window)) {
		RET_GLFW_VOID;
	}

	glfwSwapBuffers(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(swapInterval) {
	NAPI_ENV;
	REQ_INT32_ARG(0, interval);

	int normalizedInterval = normalizeSwapInterval(interval);

	rememberSwapInterval(normalizedInterval);

	GLFWwindow *window = glfwGetCurrentContext();
	auto *state = window ? reinterpret_cast<WinState *>(glfwGetWindowUserPointer(window)) : nullptr;
	glfwSwapInterval(resolveSwapInterval(state, normalizedInterval));
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(extensionSupported) {
	NAPI_ENV;
	REQ_STR_ARG(0, str);

	RET_BOOL(glfwExtensionSupported(str.c_str()) == GLFW_TRUE);
}

} // namespace glfw
