#include <iostream>
#include <chrono>

#include "glfw-common.hpp"
#include "glfw-monitors.hpp"
#include "glfw-platform.hpp"
#include "glfw-events.hpp"
#include "glfw-window.hpp"


namespace glfw {

std::vector<WinState *> states;

// Cached visibility hint value
bool isHintVisible = true;


// The default context for all to share
GLFWwindow *_share = nullptr;


static inline uint64_t getUnixTime() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}


DBG_EXPORT void dropShare() {
	_share = nullptr;
}


DBG_EXPORT void forEachWindow(const StatesIterator& fn) {
	std::for_each(
		states.begin(),
		states.end(),
		[&](WinState *state) {
			if (!state->window) {
				return;
			}
			fn(state);
		}
	);
}


// Cleanup resources
DBG_EXPORT void destroyAllWindows() {
	// Clear all callbacks
	forEachWindow([](WinState *state) {
		GLFWwindow *window = state->window;
		// Window callbacks
		glfwSetWindowPosCallback(window, nullptr);
		glfwSetWindowSizeCallback(window, nullptr);
		glfwSetWindowCloseCallback(window, nullptr);
		glfwSetWindowRefreshCallback(window, nullptr);
		glfwSetWindowFocusCallback(window, nullptr);
		glfwSetWindowIconifyCallback(window, nullptr);
		glfwSetFramebufferSizeCallback(window, nullptr);
		glfwSetDropCallback(window, nullptr);
		// Input callbacks
		glfwSetKeyCallback(window, nullptr);
		glfwSetCharCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwSetCursorPosCallback(window, nullptr);
		glfwSetCursorEnterCallback(window, nullptr);
		glfwSetScrollCallback(window, nullptr);
	});
	
	// Destroy all windows
	forEachWindow([](WinState *state) {
		glfwDestroyWindow(state->window);
		state->window = nullptr;
	});
	
	states.clear();
}


DBG_EXPORT JS_METHOD(windowHint) { NAPI_ENV;
	REQ_UINT32_ARG(0, hint);
	REQ_UINT32_ARG(1, value);
	
	if (hint == GLFW_VISIBLE) {
		isHintVisible = value != 0;
	}
	
	glfwWindowHint(hint, value);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(windowHintString) { NAPI_ENV;
	REQ_UINT32_ARG(0, hint);
	REQ_STR_ARG(1, value);
	
	glfwWindowHintString(hint, value.c_str());
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(defaultWindowHints) {
	glfwDefaultWindowHints();
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(createWindow) { NAPI_ENV;
	REQ_UINT32_ARG(0, width);
	REQ_UINT32_ARG(1, height);
	REQ_OBJ_ARG(2, emitter);
	LET_STR_ARG(3, str);
	LET_INT32_ARG(4, monitor_idx);
	LET_BOOL_ARG(5, isNoApi);
	
	GLFWmonitor **monitors = nullptr;
	GLFWmonitor *monitor = nullptr;
	int monitorCount;
	
	bool hasMonitor = info.Length() >= 5 && !(info[4].IsNull() || info[4].IsUndefined());
	
	if (hasMonitor) {
		monitors = glfwGetMonitors(&monitorCount);
		if (monitor_idx >= 0 && monitor_idx < monitorCount) {
			monitor = monitors[monitor_idx];
		} else {
			std::cerr << "Error. Ignoring invalid monitor index: " << monitor_idx << "." << std::endl;
		}
	}

// WAYLAND fix - will not be needed after 3.5? See: https://github.com/glfw/glfw/issues/2518
#ifdef __linux__
	if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
		glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	}
#endif
	
	if (!isNoApi && !_share) {
		glfwWindowHint(GLFW_VISIBLE, false);
		_share = glfwCreateWindow(128, 128, "_GLFW_ROOT_SHARED", nullptr, nullptr);
		glfwWindowHint(GLFW_VISIBLE, isHintVisible);
	}
	
	GLFWwindow *window = glfwCreateWindow(
		width,
		height,
		str.c_str(),
		monitor,
		isNoApi ? nullptr : _share
	);
	
	if (!window) {
		JS_THROW("Can't create GLFW window");
		RET_NULL;
	}
	
	if (!isNoApi) {
		glfwMakeContextCurrent(window);
	}
	
	// make sure cursor is always shown
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
	emitter.Set("__mouse", JS_OBJECT);
	emitter.Set("__key", JS_OBJECT);
	
	// Store WinState as user pointer
	WinState *state = new WinState(window, emitter);
	states.push_back(state);
	glfwSetWindowUserPointer(window, state);
	
	
	// Window callbacks
	glfwSetWindowPosCallback(window, windowPosCB);
	glfwSetWindowSizeCallback(window, windowSizeCB);
	glfwSetWindowCloseCallback(window, windowCloseCB);
	glfwSetWindowRefreshCallback(window, windowRefreshCB);
	glfwSetWindowFocusCallback(window, windowFocusCB);
	glfwSetWindowIconifyCallback(window, windowIconifyCB);
	glfwSetFramebufferSizeCallback(window, windowFramebufferSizeCB);
	glfwSetDropCallback(window, windowDropCB);
	
	// Input callbacks
	glfwSetKeyCallback(window, keyCB);
	glfwSetCharCallback(window, charCB);
	glfwSetMouseButtonCallback(window, mouseButtonCB);
	glfwSetCursorPosCallback(window, cursorPosCB);
	glfwSetCursorEnterCallback(window, cursorEnterCB);
	glfwSetScrollCallback(window, scrollCB);
	
	RET_WINDOW(window);
}


DBG_EXPORT JS_METHOD(destroyWindow) { NAPI_ENV; THIS_WINDOW; THIS_STATE;
	glfwDestroyWindow(window);
	if (state) {
		state->window = nullptr;
	}
	
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(setWindowTitle) { NAPI_ENV; THIS_WINDOW;
	REQ_STR_ARG(1, str);
	
	glfwSetWindowTitle(window, str.c_str());
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(setWindowIcon) { NAPI_ENV; THIS_WINDOW;
	REQ_OBJ_ARG(1, icon);
	
	if (!(icon.Has("width") && icon.Has("height"))) {
		RET_GLFW_VOID;
	}
	
	bool noflip = icon.Has("noflip") ? icon.Get("noflip").ToBoolean().Value() : false;
	
	GLFWimage image;
	image.width = icon.Get("width").ToNumber().Int32Value();
	image.height = icon.Get("height").ToNumber().Int32Value();
	
	int32_t numPixels = 0;
	uint8_t *src = getData<uint8_t>(env, icon, &numPixels);
	const uint64_t requiredBytes = (
		static_cast<uint64_t>(image.width) * static_cast<uint64_t>(image.height) * 4
	);
	
	if (
		image.width <= 0 ||
		image.height <= 0 ||
		src == nullptr ||
		static_cast<uint64_t>(numPixels) < requiredBytes
	) {
		JS_THROW("Icon data must contain width * height * 4 bytes.");
		RET_UNDEFINED;
	}
	
	if (noflip == false) {
		std::vector<uint8_t> dest(static_cast<size_t>(requiredBytes));
		int32_t lastY = image.height - 1;
		for (int32_t y = 0; y < image.height; y++) {
			for (int32_t x = 0; x < image.width; x++) {
				int32_t iForward = (y * image.width + x) << 2;
				int32_t iBackward = ((lastY - y) * image.width + x) << 2;
				dest[static_cast<size_t>(iForward + 0)] = src[iBackward + 0];
				dest[static_cast<size_t>(iForward + 1)] = src[iBackward + 1];
				dest[static_cast<size_t>(iForward + 2)] = src[iBackward + 2];
				dest[static_cast<size_t>(iForward + 3)] = src[iBackward + 3];
			}
		}
		image.pixels = dest.data();
		glfwSetWindowIcon(window, 1, &image);
	} else {
		image.pixels = src;
		glfwSetWindowIcon(window, 1, &image);
	}
	
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getWindowSize) { NAPI_ENV; THIS_WINDOW;
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	
	Napi::Object obj = JS_OBJECT;
	obj.Set("width", JS_NUM(w));
	obj.Set("height", JS_NUM(h));
	
	RET_VALUE(obj);
}


DBG_EXPORT JS_METHOD(getWindowFrameSize) { NAPI_ENV; THIS_WINDOW;
	int left, top, right, bottom;
	glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
	
	Napi::Object jsFrame = JS_OBJECT;
	
	jsFrame.Set("left", left);
	jsFrame.Set("top", top);
	jsFrame.Set("right", right);
	jsFrame.Set("bottom", bottom);
	
	RET_VALUE(jsFrame);
}


DBG_EXPORT JS_METHOD(getWindowContentScale) { NAPI_ENV; THIS_WINDOW;
	float xscale, yscale;
	glfwGetWindowContentScale(window, &xscale, &yscale);
	
	Napi::Object jsScale = JS_OBJECT;
	
	jsScale.Set("xscale", xscale);
	jsScale.Set("yscale", yscale);
	
	RET_VALUE(jsScale);
}


DBG_EXPORT JS_METHOD(setWindowSize) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, w);
	REQ_UINT32_ARG(2, h);
	
	glfwSetWindowSize(window, w, h);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(setWindowSizeLimits) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, minwidth);
	REQ_UINT32_ARG(2, minheight);
	REQ_UINT32_ARG(3, maxwidth);
	REQ_UINT32_ARG(4, maxheight);
	
	glfwSetWindowSizeLimits(window, minwidth, minheight, maxwidth, maxheight);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(setWindowAspectRatio) { NAPI_ENV; THIS_WINDOW;
	USE_UINT32_ARG(1, numer, GLFW_DONT_CARE);
	USE_UINT32_ARG(2, denom, GLFW_DONT_CARE);
	
	glfwSetWindowAspectRatio(window, numer, denom);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(setWindowPos) { NAPI_ENV; THIS_WINDOW;
	REQ_INT32_ARG(1, x);
	REQ_INT32_ARG(2, y);
	
	glfwSetWindowPos(window, x, y);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getWindowPos) { NAPI_ENV; THIS_WINDOW;
	int xpos, ypos;
	glfwGetWindowPos(window, &xpos, &ypos);
	
	Napi::Object obj = JS_OBJECT;
	obj.Set("x", JS_NUM(xpos));
	obj.Set("y", JS_NUM(ypos));
	
	RET_VALUE(obj);
}


DBG_EXPORT JS_METHOD(getWindowOpacity) { NAPI_ENV; THIS_WINDOW;
	float opacity = glfwGetWindowOpacity(window);
	RET_NUM(opacity);
}


DBG_EXPORT JS_METHOD(setWindowOpacity) { NAPI_ENV; THIS_WINDOW;
	REQ_FLOAT_ARG(1, opacity);
	
	glfwSetWindowOpacity(window, opacity);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(maximizeWindow) { NAPI_ENV; THIS_WINDOW;
	glfwMaximizeWindow(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(focusWindow) { NAPI_ENV; THIS_WINDOW;
	glfwFocusWindow(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(requestWindowAttention) { NAPI_ENV; THIS_WINDOW;
	glfwRequestWindowAttention(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getWindowMonitor) { NAPI_ENV; THIS_WINDOW;
	GLFWmonitor *monitor = glfwGetWindowMonitor(window);
	
	if (!monitor) {
		RET_NULL;
	}
	
	GLFWmonitor *primary = glfwGetPrimaryMonitor();
	
	RET_VALUE(describeMonitor(env, monitor, monitor == primary));
}


DBG_EXPORT JS_METHOD(iconifyWindow) { NAPI_ENV; THIS_WINDOW;
	glfwIconifyWindow(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(restoreWindow) { NAPI_ENV; THIS_WINDOW;
	glfwRestoreWindow(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(hideWindow) { NAPI_ENV; THIS_WINDOW;
	glfwHideWindow(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(showWindow) { NAPI_ENV; THIS_WINDOW;
	glfwShowWindow(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(windowShouldClose) { NAPI_ENV; THIS_WINDOW;
	RET_NUM(glfwWindowShouldClose(window));
}


DBG_EXPORT JS_METHOD(setWindowShouldClose) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, value);
	
	glfwSetWindowShouldClose(window, value);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getWindowAttrib) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, attrib);
	
	RET_NUM(glfwGetWindowAttrib(window, attrib));
}


DBG_EXPORT JS_METHOD(setWindowAttrib) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, attrib);
	REQ_UINT32_ARG(2, value);
	
	glfwSetWindowAttrib(window, attrib, value);
	
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(drawWindow) { NAPI_ENV; THIS_WINDOW;
	REQ_FUN_ARG(1, cb);
	
	glfwPollEvents();
	
	napi_value args[1] = { JS_NUM(getUnixTime()) };
	cb.Call(1, args);
	
	glfwSwapBuffers(window);
	
	RET_GLFW_VOID;
}


} // namespace glfw
