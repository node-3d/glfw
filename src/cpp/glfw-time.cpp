#include "glfw-common.hpp"
#include "glfw-time.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(getTime) {
	NAPI_ENV;
	RET_NUM(glfwGetTime());
}


DBG_EXPORT JS_METHOD(setTime) {
	NAPI_ENV;
	REQ_DOUBLE_ARG(0, time);
	glfwSetTime(time);
	RET_GLFW_VOID;
}

} // namespace glfw
