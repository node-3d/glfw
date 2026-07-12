#pragma once

#include "glfw-common.hpp"


namespace glfw {
struct WinState {
	int mouseX;
	int mouseY;
	int pendingKey;
	int pendingScan;
	int pendingAction;
	int pendingMods;
	GLFWwindow *window;
	Napi::ObjectReference emitter;
	Napi::AsyncContext context;

	WinState(GLFWwindow *_window, Napi::Object _emitter)
	    : mouseX(0), mouseY(0), pendingKey(0), pendingScan(0), pendingAction(0), pendingMods(0),
	      window(_window), context(_emitter.Env(), "GLFWEvent") {
		emitter.Reset(_emitter, 1);
	}

	~WinState() {
		emitter.Reset();
	}
};

using StatesIterator = std::function<void(WinState *)>;
DBG_EXPORT void forEachWindow(const StatesIterator &fn);

DBG_EXPORT void dropShare();
DBG_EXPORT void destroyAllWindows();

DBG_EXPORT JS_METHOD(windowHint);
DBG_EXPORT JS_METHOD(windowHintString);
DBG_EXPORT JS_METHOD(defaultWindowHints);
DBG_EXPORT JS_METHOD(createWindow);
DBG_EXPORT JS_METHOD(destroyWindow);
DBG_EXPORT JS_METHOD(setWindowTitle);
DBG_EXPORT JS_METHOD(setWindowIcon);
DBG_EXPORT JS_METHOD(getWindowSize);
DBG_EXPORT JS_METHOD(getWindowFrameSize);
DBG_EXPORT JS_METHOD(getWindowContentScale);
DBG_EXPORT JS_METHOD(setWindowSize);
DBG_EXPORT JS_METHOD(setWindowSizeLimits);
DBG_EXPORT JS_METHOD(setWindowAspectRatio);
DBG_EXPORT JS_METHOD(setWindowPos);
DBG_EXPORT JS_METHOD(getWindowPos);
DBG_EXPORT JS_METHOD(getWindowOpacity);
DBG_EXPORT JS_METHOD(setWindowOpacity);
DBG_EXPORT JS_METHOD(maximizeWindow);
DBG_EXPORT JS_METHOD(focusWindow);
DBG_EXPORT JS_METHOD(requestWindowAttention);
DBG_EXPORT JS_METHOD(getWindowMonitor);
DBG_EXPORT JS_METHOD(getFramebufferSize);
DBG_EXPORT JS_METHOD(iconifyWindow);
DBG_EXPORT JS_METHOD(restoreWindow);
DBG_EXPORT JS_METHOD(hideWindow);
DBG_EXPORT JS_METHOD(showWindow);
DBG_EXPORT JS_METHOD(windowShouldClose);
DBG_EXPORT JS_METHOD(setWindowShouldClose);
DBG_EXPORT JS_METHOD(getWindowAttrib);
DBG_EXPORT JS_METHOD(setWindowAttrib);
DBG_EXPORT JS_METHOD(drawWindow);
} // namespace glfw
