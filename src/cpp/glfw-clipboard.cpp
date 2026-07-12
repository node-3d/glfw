#include "glfw-common.hpp"
#include "glfw-clipboard.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(setClipboardString) {
	NAPI_ENV;
	THIS_WINDOW;
	REQ_STR_ARG(1, str);

	glfwSetClipboardString(window, str.c_str());
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getClipboardString) {
	NAPI_ENV;
	THIS_WINDOW;
	const char *str = glfwGetClipboardString(window);

	if (!str) {
		RET_STR("");
	}

	RET_STR(str);
}

} // namespace glfw
