#include <sstream>

#include "glfw-common.hpp"
#include "glfw-events.hpp"
#include "glfw-window.hpp"

namespace glfw {

DBG_EXPORT JS_METHOD(pollEvents) {
	glfwPollEvents();
	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(waitEvents) {
	glfwWaitEvents();
	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(waitEventsTimeout) {
	NAPI_ENV;
	REQ_DOUBLE_ARG(0, timeout);
	glfwWaitEventsTimeout(timeout);
	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(postEmptyEvent) {
	glfwPostEmptyEvent();
	RET_GLFW_VOID;
}

const char typeKeyup[] = "keyup";
const char typeKeydown[] = "keydown";
const char typeMouseenter[] = "mouseenter";
const char typeMouseleave[] = "mouseleave";
const char typeFocusin[] = "focusin";
const char typeFocusout[] = "focusout";
const char typeFocus[] = "focus";
const char typeBlur[] = "blur";

inline void _emit(WinState *state, const char *name, Napi::Value argv) {
	if (!state->window) {
		return;
	}

	eventEmit(state->emitter.Value(), name, 1, &argv, state->context);
}

void fillMouse(Napi::Object evt, GLFWwindow *window, int mods = -1) {
	Napi::Env env = evt.Env();

	WinState *state = reinterpret_cast<WinState *>(glfwGetWindowUserPointer(window));

	int buttons = 0;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
		buttons |= 1;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
		buttons |= 2;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE)) {
		buttons |= 4;
	}
	evt.Set("buttons", JS_NUM(buttons));

	evt.Set("clientX", JS_NUM(state->mouseX));
	evt.Set("clientY", JS_NUM(state->mouseY));
	evt.Set("pageX", JS_NUM(state->mouseX));
	evt.Set("pageY", JS_NUM(state->mouseY));
	evt.Set("x", JS_NUM(state->mouseX));
	evt.Set("y", JS_NUM(state->mouseY));

	if (mods > -1) {
		evt.Set("shiftKey", static_cast<bool>(mods & GLFW_MOD_SHIFT));
		evt.Set("ctrlKey", static_cast<bool>(mods & GLFW_MOD_CONTROL));
		evt.Set("altKey", static_cast<bool>(mods & GLFW_MOD_ALT));
		evt.Set("metaKey", static_cast<bool>(mods & GLFW_MOD_SUPER));
	} else {
		evt.Set(
		    "ctrlKey",
		    static_cast<bool>(
		        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS
		    )
		);
		evt.Set(
		    "shiftKey",
		    static_cast<bool>(
		        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
		    )
		);
		evt.Set(
		    "altKey",
		    static_cast<bool>(
		        glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
		        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS
		    )
		);
		evt.Set(
		    "metaKey",
		    static_cast<bool>(
		        glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
		        glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS
		    )
		);
	}
}

void fillKey(Napi::Object evt, int glfwKey, int scancode, int action, int mods) {
	Napi::Env env = evt.Env();

	const char *keyName = glfwGetKeyName(glfwKey, scancode);

	evt.Set("repeat", static_cast<bool>(action == GLFW_REPEAT));
	evt.Set("altKey", static_cast<bool>(mods & GLFW_MOD_ALT));
	evt.Set("ctrlKey", static_cast<bool>(mods & GLFW_MOD_CONTROL));
	evt.Set("metaKey", static_cast<bool>(mods & GLFW_MOD_SUPER));
	evt.Set("shiftKey", static_cast<bool>(mods & GLFW_MOD_SHIFT));

	if (keyName) {
		evt.Set("code", keyName);
		evt.Set("key", keyName);
	} else {
		evt.Set("code", JS_NULL);
		evt.Set("key", JS_NULL);
	}

	evt.Set("which", JS_NUM(glfwKey));
}

void windowPosCB(GLFWwindow *window, int xpos, int ypos) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	evt.Set("type", "move");
	evt.Set("x", JS_NUM(xpos));
	evt.Set("y", JS_NUM(ypos));

	_emit(state, "move", evt);
}

void windowSizeCB(GLFWwindow *window, int w, int h) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	evt.Set("type", "resize");
	evt.Set("width", JS_NUM(w));
	evt.Set("height", JS_NUM(h));

	_emit(state, "wresize", evt);
}

void windowFramebufferSizeCB(GLFWwindow *window, int w, int h) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	evt.Set("type", "resize");
	evt.Set("width", JS_NUM(w));
	evt.Set("height", JS_NUM(h));

	_emit(state, "resize", evt);
}

void windowDropCB(GLFWwindow *window, int count, const char **paths) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	fillMouse(evt, window);
	evt.Set("type", "drop");

	Napi::Object dataTransfer = JS_OBJECT;
	evt.Set("dataTransfer", dataTransfer);

	Napi::Array list = JS_ARRAY;
	for (int i = 0; i < count; i++) {
		list.Set(i, paths[i]);
	}

	evt.Set("dropEffect", "none");
	evt.Set("effectAllowed", "all");
	evt.Set("files", list);
	evt.Set("items", list);
	evt.Set("types", JS_ARRAY);

	_emit(state, "drop", evt);
}

void windowCloseCB(GLFWwindow *window) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	evt.Set("type", "quit");

	_emit(state, "quit", evt);

	state->window = nullptr;
}

void windowRefreshCB(GLFWwindow *window) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	evt.Set("type", "refresh");

	_emit(state, "refresh", evt);
}

void windowIconifyCB(GLFWwindow *window, int iconified) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	evt.Set("type", "iconify");
	evt.Set("iconified", static_cast<bool>(iconified));

	_emit(state, "iconify", evt);
}

void windowFocusCB(GLFWwindow *window, int focused) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	const char *typeFound;
	const char *typeDirFound;
	if (focused) {
		typeFound = typeFocus;
		typeDirFound = typeFocusin;
	} else {
		typeFound = typeBlur;
		typeDirFound = typeFocusout;
	}

	Napi::Object evt1 = JS_OBJECT;
	evt1.Set("type", typeFound);
	_emit(state, typeFound, evt1);

	Napi::Object evt2 = JS_OBJECT;
	evt2.Set("type", typeDirFound);
	_emit(state, typeDirFound, evt2);
}

void keyCB(GLFWwindow *window, int glfwKey, int scancode, int action, int mods) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	const char *keyName = glfwGetKeyName(glfwKey, scancode);

	bool isAltPressed = mods & GLFW_MOD_ALT;
	bool isCtrlPressed = mods & GLFW_MOD_CONTROL;

	if (keyName && (action == GLFW_PRESS || action == GLFW_REPEAT) && !(isAltPressed || isCtrlPressed)) {
		WinState *state = reinterpret_cast<WinState *>(glfwGetWindowUserPointer(window));
		state->pendingKey = glfwKey;
		state->pendingScan = scancode;
		state->pendingAction = action;
		state->pendingMods = mods;
		return;
	}

	const char *typeFound;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		typeFound = typeKeydown;
	} else {
		typeFound = typeKeyup;
	}

	Napi::Object evt = state->emitter.Get("__key").As<Napi::Object>();
	fillKey(evt, glfwKey, scancode, action, mods);
	evt.Set("charCode", JS_NUM(0));
	evt.Set("type", typeFound);

	_emit(state, typeFound, evt);
}

void charCB(GLFWwindow *window, unsigned codepoint) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	if (!state->pendingKey) {
		return;
	}

	Napi::Object evt = state->emitter.Get("__key").As<Napi::Object>();
	fillKey(evt, state->pendingKey, state->pendingScan, state->pendingAction, state->pendingMods);
	evt.Set("charCode", JS_NUM(codepoint));
	evt.Set("type", typeKeydown);

	state->pendingKey = 0;
	state->pendingScan = 0;
	state->pendingAction = 0;
	state->pendingMods = 0;

	_emit(state, typeKeydown, evt);
}

void cursorPosCB(GLFWwindow *window, double x, double y) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	int newX = static_cast<int>(x);
	int newY = static_cast<int>(y);

	Napi::Object evt = state->emitter.Get("__mouse").As<Napi::Object>();
	evt.Set("movementX", JS_NUM(newX - state->mouseX));
	evt.Set("movementY", JS_NUM(newY - state->mouseY));

	state->mouseX = newX;
	state->mouseY = newY;

	fillMouse(evt, window);
	evt.Set("type", "mousemove");

	_emit(state, "mousemove", evt);
}

void cursorEnterCB(GLFWwindow *window, int entered) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	const char *typeFound;
	if (entered) {
		typeFound = typeMouseenter;
	} else {
		typeFound = typeMouseleave;
	}

	Napi::Object evt = JS_OBJECT;
	fillMouse(evt, window);
	evt.Set("type", typeFound);

	_emit(state, typeFound, evt);
}

void mouseButtonCB(GLFWwindow *window, int button, int action, int mods) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	int btnId = button;
	if (button == GLFW_MOUSE_BUTTON_2) {
		btnId = 2;
	} else if (button == GLFW_MOUSE_BUTTON_3) {
		btnId = 1;
	}
	int which = btnId + 1;

	Napi::Object evt1 = JS_OBJECT;
	fillMouse(evt1, window);
	evt1.Set("type", action ? "mousedown" : "mouseup");
	evt1.Set("button", JS_NUM(btnId));

	evt1.Set("which", JS_NUM(which));

	_emit(state, action ? "mousedown" : "mouseup", evt1);

	if (!action) {
		Napi::Object evt2 = JS_OBJECT;
		fillMouse(evt2, window);
		evt2.Set("type", "click");
		evt2.Set("button", JS_NUM(btnId));
		evt2.Set("which", JS_NUM(which));

		_emit(state, "click", evt2);
	}
}

void scrollCB(GLFWwindow *window, double xoffset, double yoffset) {
	THIS_STATE;
	STATE_ENV;
	NAPI_HS;

	Napi::Object evt = JS_OBJECT;
	fillMouse(evt, window);
	evt.Set("type", "wheel");
	evt.Set("deltaX", JS_NUM(static_cast<int>(xoffset * 100)));
	evt.Set("deltaY", JS_NUM(static_cast<int>(yoffset * 100)));
	evt.Set("deltaZ", JS_NUM(0));
	evt.Set("wheelDeltaX", JS_NUM(static_cast<int>(xoffset * 120)));
	evt.Set("wheelDeltaY", JS_NUM(static_cast<int>(yoffset * 120)));
	evt.Set("wheelDelta", JS_NUM(static_cast<int>(yoffset * 120)));

	_emit(state, "wheel", evt);
	_emit(state, "mousewheel", evt);
}

void joystickCb(int jid, int event) {
	// Emit to all windows
	forEachWindow([jid, event](WinState *state) {
		STATE_ENV;
		NAPI_HS;
		Napi::Object evt = JS_OBJECT;
		evt.Set("type", "joystick");
		evt.Set("id", JS_NUM(jid));
		evt.Set("event", JS_NUM(event)); // GLFW_CONNECTED or GLFW_DISCONNECTED
		_emit(state, "joystick", evt);
	});
}

} // namespace glfw
