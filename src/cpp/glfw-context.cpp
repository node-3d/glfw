#include "glfw-common.hpp"
#include "glfw-context.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(makeContextCurrent) { NAPI_ENV; THIS_WINDOW;
	glfwMakeContextCurrent(window);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(getCurrentContext) { NAPI_ENV;
	GLFWwindow *window = glfwGetCurrentContext();
	RET_PTR(window);
}

} // namespace glfw
