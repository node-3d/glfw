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

constexpr GLenum GL_FRAMEBUFFER_EXT_VALUE = 0x8D40;
constexpr GLenum GL_COLOR_ATTACHMENT0_EXT_VALUE = 0x8CE0;
constexpr GLenum GL_FRAMEBUFFER_COMPLETE_EXT_VALUE = 0x8CD5;

using GlGenFramebuffers = void (*)(GLsizei n, GLuint *framebuffers);
using GlBindFramebuffer = void (*)(GLenum target, GLuint framebuffer);
using GlFramebufferTexture2D =
    void (*)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
using GlCheckFramebufferStatus = GLenum (*)(GLenum target);
using GlDeleteFramebuffers = void (*)(GLsizei n, const GLuint *framebuffers);

template <typename T> T getGlProc(const char *name) {
	return reinterpret_cast<T>(glfwGetProcAddress(name));
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

DBG_EXPORT JS_METHOD(testHeadlessFbo) {
	NAPI_ENV;
	USE_UINT32_ARG(0, width, 64);
	USE_UINT32_ARG(1, height, 64);

	if (!isInited) {
		setlocale(LC_ALL, "");
		glfwSetErrorCallback(errorCb);
		isInited = glfwInit() == GLFW_TRUE;
	}

	if (!isInited) {
		JS_THROW("Failed to initialize GLFW");
		RET_NULL;
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_SAMPLES, 0);

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	GLFWwindow *window = glfwCreateWindow(width, height, "node-3d-headless-fbo", nullptr, nullptr);
	if (!window) {
		JS_THROW("Failed to create GLFW headless FBO window");
		RET_NULL;
	}

	glfwMakeContextCurrent(window);

	auto glGenFramebuffersPtr = getGlProc<GlGenFramebuffers>("glGenFramebuffers");
	auto glBindFramebufferPtr = getGlProc<GlBindFramebuffer>("glBindFramebuffer");
	auto glFramebufferTexture2DPtr = getGlProc<GlFramebufferTexture2D>("glFramebufferTexture2D");
	auto glCheckFramebufferStatusPtr = getGlProc<GlCheckFramebufferStatus>("glCheckFramebufferStatus");
	auto glDeleteFramebuffersPtr = getGlProc<GlDeleteFramebuffers>("glDeleteFramebuffers");

	if (!glGenFramebuffersPtr || !glBindFramebufferPtr || !glFramebufferTexture2DPtr ||
	    !glCheckFramebufferStatusPtr || !glDeleteFramebuffersPtr) {
		glfwDestroyWindow(window);
		JS_THROW("Framebuffer OpenGL functions are not available");
		RET_NULL;
	}

	GLuint fbo = 0;
	GLuint texture = 0;

	glGenFramebuffersPtr(1, &fbo);
	glBindFramebufferPtr(GL_FRAMEBUFFER_EXT_VALUE, fbo);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2DPtr(
	    GL_FRAMEBUFFER_EXT_VALUE, GL_COLOR_ATTACHMENT0_EXT_VALUE, GL_TEXTURE_2D, texture, 0
	);

	GLenum status = glCheckFramebufferStatusPtr(GL_FRAMEBUFFER_EXT_VALUE);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT_VALUE) {
		glDeleteTextures(1, &texture);
		glDeleteFramebuffersPtr(1, &fbo);
		glfwDestroyWindow(window);
		JS_THROW("Framebuffer is incomplete");
		RET_NULL;
	}

	glViewport(0, 0, width, height);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	std::vector<uint8_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::Copy(env, pixels.data(), pixels.size());
	Napi::Object result = JS_OBJECT;
	result.Set("width", width);
	result.Set("height", height);
	result.Set("status", status);
	result.Set("pixels", buffer);
	result.Set("vendor", stringOrEmpty(glGetString(GL_VENDOR)));
	result.Set("renderer", stringOrEmpty(glGetString(GL_RENDERER)));
	result.Set("version", stringOrEmpty(glGetString(GL_VERSION)));

	glBindFramebufferPtr(GL_FRAMEBUFFER_EXT_VALUE, 0);
	glDeleteTextures(1, &texture);
	glDeleteFramebuffersPtr(1, &fbo);
	glfwDestroyWindow(window);

	RET_VALUE(result);
}

} // namespace glfw
