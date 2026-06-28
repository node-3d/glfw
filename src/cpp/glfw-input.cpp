#include "glfw-common.hpp"
#include "glfw-input.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(setInputMode) { NAPI_ENV; THIS_WINDOW;
	REQ_INT32_ARG(1, mode);
	REQ_INT32_ARG(2, value);
	
	glfwSetInputMode(window, mode, value);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getInputMode) { NAPI_ENV; THIS_WINDOW;
	REQ_INT32_ARG(1, mode);
	RET_NUM(glfwGetInputMode(window, mode));
}


/* Input handling */
DBG_EXPORT JS_METHOD(getKey) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, key);
	RET_NUM(glfwGetKey(window, key));
}


DBG_EXPORT JS_METHOD(getMouseButton) { NAPI_ENV; THIS_WINDOW;
	REQ_UINT32_ARG(1, button);
	RET_NUM(glfwGetMouseButton(window, button));
}


DBG_EXPORT JS_METHOD(rawMouseMotionSupported) { NAPI_ENV;
	RET_BOOL(glfwRawMouseMotionSupported() == GLFW_TRUE);
}


DBG_EXPORT JS_METHOD(getKeyName) { NAPI_ENV;
	REQ_INT32_ARG(0, key);
	REQ_INT32_ARG(1, scancode);
	
	const char *name = glfwGetKeyName(key, scancode);
	if (!name) {
		RET_NULL;
	}
	
	RET_STR(name);
}


DBG_EXPORT JS_METHOD(getKeyScancode) { NAPI_ENV;
	REQ_INT32_ARG(0, key);
	RET_NUM(glfwGetKeyScancode(key));
}


} // namespace glfw
