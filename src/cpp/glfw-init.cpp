#include <iostream>
#include <locale.h>
#include <vector>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#endif

#include "glfw-common.hpp"
#include "glfw-init.hpp"
#include "glfw-window.hpp"
#include "glfw-events.hpp"

namespace glfw {

bool isInited = false;

using GlViewport = void (*)(GLint x, GLint y, GLsizei width, GLsizei height);
using GlClearColor = void (*)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
using GlClear = void (*)(GLbitfield mask);
using GlReadPixels =
    void (*)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
using GlGetString = const GLubyte *(*)(GLenum name);

DBG_EXPORT void errorCb(int error, const char *description);

template <typename T> T getGlProc(const char *name, T fallback) {
	T symbol = reinterpret_cast<T>(glfwGetProcAddress(name));
	return symbol ? symbol : fallback;
}

static inline const char *stringOrEmpty(const GLubyte *value) {
	return value ? reinterpret_cast<const char *>(value) : "";
}

DBG_EXPORT void errorCb(int error, const char *description) {
	std::cout << "GLFW Error " << error << ": " << description << std::endl;
}

// Cleanup resources
DBG_EXPORT void deinit() {
	if (!isInited) {
		return;
	}

	destroyAllWindows();

	glfwSetJoystickCallback(nullptr);

	isInited = false;
	dropShare();

	glfwTerminate();
}

DBG_EXPORT JS_METHOD(init) {
	NAPI_ENV;
	setlocale(LC_ALL, "");

	undefined = JS_UNDEFINED;

	glfwSetErrorCallback(errorCb);

	isInited = glfwInit() == GLFW_TRUE;

	glfwSetJoystickCallback(joystickCb);

	RET_BOOL(isInited);
}

DBG_EXPORT JS_METHOD(initHint) {
	NAPI_ENV;
	REQ_INT32_ARG(0, hint);
	REQ_INT32_ARG(1, value);

	glfwInitHint(hint, value);
	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(terminate) {
	deinit();
	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(testScene) {
	NAPI_ENV;
	WEAK_UINT32_ARG(0, width);
	WEAK_UINT32_ARG(1, height);
	LET_FLOAT_ARG(2, z);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	float ratio = static_cast<float>(width) / static_cast<float>(height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glRotatef(static_cast<float>(glfwGetTime()) * 50.f, 0.f, 0.f, 1.f);

	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.6f + z, -0.4f, 0.f);
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.6f + z, -0.4f, 0.f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f + z, 0.6f, 0.f);
	glEnd();

	RET_GLFW_VOID;
}

DBG_EXPORT JS_METHOD(testCurrentContextFrame) {
	NAPI_ENV;
	USE_UINT32_ARG(0, width, 64);
	USE_UINT32_ARG(1, height, 64);

	GLFWwindow *window = glfwGetCurrentContext();
	if (!window) {
		JS_THROW("No current GLFW context");
		RET_NULL;
	}

	auto glViewportPtr = getGlProc<GlViewport>("glViewport", glViewport);
	auto glClearColorPtr = getGlProc<GlClearColor>("glClearColor", glClearColor);
	auto glClearPtr = getGlProc<GlClear>("glClear", glClear);
	auto glReadPixelsPtr = getGlProc<GlReadPixels>("glReadPixels", glReadPixels);
	auto glGetStringPtr = getGlProc<GlGetString>("glGetString", glGetString);

	if (!glViewportPtr || !glClearColorPtr || !glClearPtr || !glReadPixelsPtr || !glGetStringPtr) {
		JS_THROW("Required OpenGL entry points are not available");
		RET_NULL;
	}

	glViewportPtr(0, 0, width, height);
	glClearColorPtr(1.0f, 0.0f, 0.0f, 1.0f);
	glClearPtr(GL_COLOR_BUFFER_BIT);

	std::vector<uint8_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
	glReadPixelsPtr(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::Copy(env, pixels.data(), pixels.size());
	Napi::Object result = JS_OBJECT;
	result.Set("width", width);
	result.Set("height", height);
	result.Set("pixels", buffer);
	result.Set("vendor", stringOrEmpty(glGetStringPtr(GL_VENDOR)));
	result.Set("renderer", stringOrEmpty(glGetStringPtr(GL_RENDERER)));
	result.Set("version", stringOrEmpty(glGetStringPtr(GL_VERSION)));

	RET_VALUE(result);
}

} // namespace glfw
