#include "glfw-console.hpp"
#include "glfw-platform.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(hideConsole) {
#ifdef _WIN32
	auto wnd = FindWindowA("ConsoleWindowClass", nullptr);
	ShowWindow(wnd, 0);
#endif
	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(showConsole) {
#ifdef _WIN32
	auto wnd = FindWindowA("ConsoleWindowClass", nullptr);
	ShowWindow(wnd, 1);
#endif
	RET_GLFW_VOID;
}

} // namespace glfw
