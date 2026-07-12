#include "glfw-common.hpp"
#include "glfw-timers.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(getTimerValue) {
	NAPI_ENV;
	RET_NUM(glfwGetTimerValue());
}


DBG_EXPORT JS_METHOD(getTimerFrequency) {
	NAPI_ENV;
	RET_NUM(glfwGetTimerFrequency());
}

} // namespace glfw
