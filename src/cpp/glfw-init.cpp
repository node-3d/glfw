#include <iostream>
#include <locale.h>
#include <vector>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#include <cstdint>
#include <cstdlib>
#include <dlfcn.h>
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
constexpr GLint GL_RGBA8_VALUE = 0x8058;

using GlGenFramebuffers = void (*)(GLsizei n, GLuint *framebuffers);
using GlBindFramebuffer = void (*)(GLenum target, GLuint framebuffer);
using GlFramebufferTexture2D =
    void (*)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
using GlCheckFramebufferStatus = GLenum (*)(GLenum target);
using GlDeleteFramebuffers = void (*)(GLsizei n, const GLuint *framebuffers);
using GlGenTextures = void (*)(GLsizei n, GLuint *textures);
using GlBindTexture = void (*)(GLenum target, GLuint texture);
using GlTexParameteri = void (*)(GLenum target, GLenum pname, GLint param);
using GlTexImage2D = void (*)(
    GLenum target,
    GLint level,
    GLint internalformat,
    GLsizei width,
    GLsizei height,
    GLint border,
    GLenum format,
    GLenum type,
    const void *pixels
);
using GlViewport = void (*)(GLint x, GLint y, GLsizei width, GLsizei height);
using GlClearColor = void (*)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
using GlClear = void (*)(GLbitfield mask);
using GlReadPixels =
    void (*)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
using GlDeleteTextures = void (*)(GLsizei n, const GLuint *textures);
using GlGetString = const GLubyte *(*)(GLenum name);
using GlGetError = GLenum (*)();

DBG_EXPORT void errorCb(int error, const char *description);

template <typename T> T getGlProc(const char *name) {
	return reinterpret_cast<T>(glfwGetProcAddress(name));
}

static inline const char *stringOrEmpty(const GLubyte *value) {
	return value ? reinterpret_cast<const char *>(value) : "";
}

#ifdef __APPLE__
using EglDisplay = void *;
using EglConfig = void *;
using EglSurface = void *;
using EglContext = void *;
using EglAttrib = intptr_t;
using EglInt = int;
using EglBoolean = unsigned int;
using EglEnum = unsigned int;
using EglProc = void (*)();

constexpr EglInt EGL_FALSE_VALUE = 0;
constexpr EglInt EGL_TRUE_VALUE = 1;
constexpr EglInt EGL_PBUFFER_BIT_VALUE = 0x0001;
constexpr EglInt EGL_OPENGL_ES2_BIT_VALUE = 0x0004;
constexpr EglInt EGL_WIDTH_VALUE = 0x3057;
constexpr EglInt EGL_HEIGHT_VALUE = 0x3056;
constexpr EglInt EGL_NONE_VALUE = 0x3038;
constexpr EglInt EGL_RED_SIZE_VALUE = 0x3024;
constexpr EglInt EGL_GREEN_SIZE_VALUE = 0x3023;
constexpr EglInt EGL_BLUE_SIZE_VALUE = 0x3022;
constexpr EglInt EGL_ALPHA_SIZE_VALUE = 0x3021;
constexpr EglInt EGL_SURFACE_TYPE_VALUE = 0x3033;
constexpr EglInt EGL_RENDERABLE_TYPE_VALUE = 0x3040;
constexpr EglInt EGL_CONTEXT_CLIENT_VERSION_VALUE = 0x3098;
constexpr EglEnum EGL_OPENGL_ES_API_VALUE = 0x30A0;
constexpr EglEnum EGL_EXTENSIONS_VALUE = 0x3055;
constexpr EglEnum EGL_PLATFORM_SURFACELESS_MESA_VALUE = 0x31DD;

using EglGetProcAddress = EglProc (*)(const char *procname);
using EglGetDisplay = EglDisplay (*)(void *displayId);
using EglGetPlatformDisplayExt =
    EglDisplay (*)(EglEnum platform, void *nativeDisplay, const EglAttrib *attribs);
using EglInitialize = EglBoolean (*)(EglDisplay display, EglInt *major, EglInt *minor);
using EglQueryString = const char *(*)(EglDisplay display, EglInt name);
using EglBindApi = EglBoolean (*)(EglEnum api);
using EglChooseConfig = EglBoolean (*)(
    EglDisplay display, const EglInt *attribs, EglConfig *configs, EglInt configSize, EglInt *numConfig
);
using EglCreatePbufferSurface = EglSurface (*)(EglDisplay display, EglConfig config, const EglInt *attribs);
using EglCreateContext =
    EglContext (*)(EglDisplay display, EglConfig config, EglContext shareContext, const EglInt *attribs);
using EglMakeCurrent =
    EglBoolean (*)(EglDisplay display, EglSurface draw, EglSurface read, EglContext context);
using EglDestroySurface = EglBoolean (*)(EglDisplay display, EglSurface surface);
using EglDestroyContext = EglBoolean (*)(EglDisplay display, EglContext context);
using EglTerminate = EglBoolean (*)(EglDisplay display);
using EglGetError = EglInt (*)();

static inline std::string getRuntimeLibraryPath(const char *name) {
	const char *runtimeDir = std::getenv("NODE_3D_GLFW_RUNTIME_LIB");
	if (runtimeDir && runtimeDir[0] != '\0') {
		return std::string(runtimeDir) + "/" + name;
	}

	return name;
}

template <typename T> T getSymbol(void *handle, const char *name) {
	return reinterpret_cast<T>(dlsym(handle, name));
}

template <typename T> T getGlSymbol(void *handle, EglGetProcAddress eglGetProcAddressPtr, const char *name) {
	T symbol = getSymbol<T>(handle, name);
	if (symbol) {
		return symbol;
	}

	return reinterpret_cast<T>(eglGetProcAddressPtr(name));
}

static inline bool hasExtension(const char *extensions, const char *needle) {
	if (!extensions || !needle) {
		return false;
	}

	const std::string all = extensions;
	const std::string current = needle;
	size_t offset = 0;
	while ((offset = all.find(current, offset)) != std::string::npos) {
		const bool startsOk = offset == 0 || all[offset - 1] == ' ';
		const size_t end = offset + current.size();
		const bool endsOk = end == all.size() || all[end] == ' ';
		if (startsOk && endsOk) {
			return true;
		}
		offset = end;
	}

	return false;
}
#endif

static inline bool isMode(const std::string &mode, const char *value) {
	return mode == value;
}

static inline void applyHeadlessFboInitHints(const std::string &mode) {
#ifdef __APPLE__
	glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);
	glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

	if (isMode(mode, "null-native") || isMode(mode, "null-egl") || isMode(mode, "null-osmesa")) {
		glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);
	}

	if (isMode(mode, "angle-d3d11")) {
		glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, GLFW_ANGLE_PLATFORM_TYPE_D3D11);
	} else if (isMode(mode, "angle-vulkan")) {
		glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, GLFW_ANGLE_PLATFORM_TYPE_VULKAN);
	} else if (isMode(mode, "angle-metal")) {
		glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, GLFW_ANGLE_PLATFORM_TYPE_METAL);
	}
}

static inline void applyHeadlessFboWindowHints(const std::string &mode) {
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_SAMPLES, 0);

	if (isMode(mode, "null-egl") || isMode(mode, "angle-d3d11") || isMode(mode, "angle-vulkan") ||
	    isMode(mode, "angle-metal") || isMode(mode, "gles-egl")) {
		glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	} else if (isMode(mode, "null-osmesa")) {
		glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
	}

	if (isMode(mode, "angle-d3d11") || isMode(mode, "angle-vulkan") || isMode(mode, "angle-metal") ||
	    isMode(mode, "gles-egl")) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		return;
	}

	if (isMode(mode, "native-loose") || isMode(mode, "null-native")) {
		return;
	}

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
}

static inline Napi::Value
runHeadlessFbo(Napi::Env env, uint32_t width, uint32_t height, const std::string &mode) {
	if (!isInited) {
		setlocale(LC_ALL, "");
		glfwSetErrorCallback(errorCb);
		applyHeadlessFboInitHints(mode);
		isInited = glfwInit() == GLFW_TRUE;
	}

	if (!isInited) {
		Napi::Error::New(env, "Failed to initialize GLFW headless FBO mode `" + mode + "`")
		    .ThrowAsJavaScriptException();
		return env.Null();
	}

	applyHeadlessFboWindowHints(mode);

	GLFWwindow *window = glfwCreateWindow(width, height, "node-3d-headless-fbo", nullptr, nullptr);
	if (!window) {
		Napi::Error::New(env, "Failed to create GLFW headless FBO window in mode `" + mode + "`")
		    .ThrowAsJavaScriptException();
		return env.Null();
	}

	glfwMakeContextCurrent(window);

	auto glGenFramebuffersPtr = getGlProc<GlGenFramebuffers>("glGenFramebuffers");
	auto glBindFramebufferPtr = getGlProc<GlBindFramebuffer>("glBindFramebuffer");
	auto glFramebufferTexture2DPtr = getGlProc<GlFramebufferTexture2D>("glFramebufferTexture2D");
	auto glCheckFramebufferStatusPtr = getGlProc<GlCheckFramebufferStatus>("glCheckFramebufferStatus");
	auto glDeleteFramebuffersPtr = getGlProc<GlDeleteFramebuffers>("glDeleteFramebuffers");
	auto glGenTexturesPtr = getGlProc<GlGenTextures>("glGenTextures");
	auto glBindTexturePtr = getGlProc<GlBindTexture>("glBindTexture");
	auto glTexParameteriPtr = getGlProc<GlTexParameteri>("glTexParameteri");
	auto glTexImage2DPtr = getGlProc<GlTexImage2D>("glTexImage2D");
	auto glViewportPtr = getGlProc<GlViewport>("glViewport");
	auto glClearColorPtr = getGlProc<GlClearColor>("glClearColor");
	auto glClearPtr = getGlProc<GlClear>("glClear");
	auto glReadPixelsPtr = getGlProc<GlReadPixels>("glReadPixels");
	auto glDeleteTexturesPtr = getGlProc<GlDeleteTextures>("glDeleteTextures");
	auto glGetStringPtr = getGlProc<GlGetString>("glGetString");
	auto glGetErrorPtr = getGlProc<GlGetError>("glGetError");

	if (!glGenTexturesPtr) {
		glGenTexturesPtr = glGenTextures;
	}
	if (!glBindTexturePtr) {
		glBindTexturePtr = glBindTexture;
	}
	if (!glTexParameteriPtr) {
		glTexParameteriPtr = glTexParameteri;
	}
	if (!glTexImage2DPtr) {
		glTexImage2DPtr = glTexImage2D;
	}
	if (!glViewportPtr) {
		glViewportPtr = glViewport;
	}
	if (!glClearColorPtr) {
		glClearColorPtr = glClearColor;
	}
	if (!glClearPtr) {
		glClearPtr = glClear;
	}
	if (!glReadPixelsPtr) {
		glReadPixelsPtr = glReadPixels;
	}
	if (!glDeleteTexturesPtr) {
		glDeleteTexturesPtr = glDeleteTextures;
	}
	if (!glGetStringPtr) {
		glGetStringPtr = glGetString;
	}
	if (!glGetErrorPtr) {
		glGetErrorPtr = glGetError;
	}

	if (!glGenFramebuffersPtr || !glBindFramebufferPtr || !glFramebufferTexture2DPtr ||
	    !glCheckFramebufferStatusPtr || !glDeleteFramebuffersPtr || !glGenTexturesPtr || !glBindTexturePtr ||
	    !glTexParameteriPtr || !glTexImage2DPtr || !glViewportPtr || !glClearColorPtr || !glClearPtr ||
	    !glReadPixelsPtr || !glDeleteTexturesPtr || !glGetStringPtr || !glGetErrorPtr) {
		glfwDestroyWindow(window);
		Napi::Error::New(env, "Framebuffer OpenGL functions are not available in mode `" + mode + "`")
		    .ThrowAsJavaScriptException();
		return env.Null();
	}

	GLuint fbo = 0;
	GLuint texture = 0;

	glGenFramebuffersPtr(1, &fbo);
	glBindFramebufferPtr(GL_FRAMEBUFFER_EXT_VALUE, fbo);

	glGenTexturesPtr(1, &texture);
	glBindTexturePtr(GL_TEXTURE_2D, texture);
	glTexParameteriPtr(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteriPtr(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteriPtr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteriPtr(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2DPtr(GL_TEXTURE_2D, 0, GL_RGBA8_VALUE, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2DPtr(
	    GL_FRAMEBUFFER_EXT_VALUE, GL_COLOR_ATTACHMENT0_EXT_VALUE, GL_TEXTURE_2D, texture, 0
	);

	GLenum glError = glGetErrorPtr();
	GLenum status = glCheckFramebufferStatusPtr(GL_FRAMEBUFFER_EXT_VALUE);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT_VALUE) {
		glDeleteTexturesPtr(1, &texture);
		glDeleteFramebuffersPtr(1, &fbo);
		glfwDestroyWindow(window);
		Napi::Error::New(
		    env,
		    "Framebuffer is incomplete in mode `" + mode + "`, status " + std::to_string(status) +
		        ", glError " + std::to_string(glError)
		)
		    .ThrowAsJavaScriptException();
		return env.Null();
	}

	glViewportPtr(0, 0, width, height);
	glClearColorPtr(1.0f, 0.0f, 0.0f, 1.0f);
	glClearPtr(GL_COLOR_BUFFER_BIT);

	std::vector<uint8_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
	glReadPixelsPtr(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::Copy(env, pixels.data(), pixels.size());
	Napi::Object result = JS_OBJECT;
	result.Set("mode", mode);
	result.Set("platform", glfwGetPlatform());
	result.Set("width", width);
	result.Set("height", height);
	result.Set("status", status);
	result.Set("pixels", buffer);
	result.Set("vendor", stringOrEmpty(glGetStringPtr(GL_VENDOR)));
	result.Set("renderer", stringOrEmpty(glGetStringPtr(GL_RENDERER)));
	result.Set("version", stringOrEmpty(glGetStringPtr(GL_VERSION)));

	glBindFramebufferPtr(GL_FRAMEBUFFER_EXT_VALUE, 0);
	glDeleteTexturesPtr(1, &texture);
	glDeleteFramebuffersPtr(1, &fbo);
	glfwDestroyWindow(window);

	return result;
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

	RET_VALUE(runHeadlessFbo(env, width, height, "default"));
}

DBG_EXPORT JS_METHOD(testHeadlessFboMode) {
	NAPI_ENV;
	USE_STR_ARG(0, mode, "default");
	USE_UINT32_ARG(1, width, 64);
	USE_UINT32_ARG(2, height, 64);

	RET_VALUE(runHeadlessFbo(env, width, height, mode));
}

DBG_EXPORT JS_METHOD(testSurfacelessEgl) {
	NAPI_ENV;
	USE_UINT32_ARG(0, width, 64);
	USE_UINT32_ARG(1, height, 64);

#ifndef __APPLE__
	JS_THROW("Surfaceless EGL diagnostic is macOS-only for now");
	RET_NULL;
#else
	std::string eglPath = getRuntimeLibraryPath("libEGL.dylib");
	std::string glesPath = getRuntimeLibraryPath("libGLESv2.dylib");
	void *eglLib = dlopen(eglPath.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (!eglLib) {
		JS_THROW(std::string("Unable to load ") + eglPath + ": " + dlerror());
		RET_NULL;
	}

	void *glesLib = dlopen(glesPath.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (!glesLib) {
		std::string error = dlerror();
		dlclose(eglLib);
		JS_THROW(std::string("Unable to load ") + glesPath + ": " + error);
		RET_NULL;
	}

	auto eglGetProcAddressPtr = getSymbol<EglGetProcAddress>(eglLib, "eglGetProcAddress");
	auto eglGetDisplayPtr = getSymbol<EglGetDisplay>(eglLib, "eglGetDisplay");
	auto eglInitializePtr = getSymbol<EglInitialize>(eglLib, "eglInitialize");
	auto eglQueryStringPtr = getSymbol<EglQueryString>(eglLib, "eglQueryString");
	auto eglBindApiPtr = getSymbol<EglBindApi>(eglLib, "eglBindAPI");
	auto eglChooseConfigPtr = getSymbol<EglChooseConfig>(eglLib, "eglChooseConfig");
	auto eglCreatePbufferSurfacePtr = getSymbol<EglCreatePbufferSurface>(eglLib, "eglCreatePbufferSurface");
	auto eglCreateContextPtr = getSymbol<EglCreateContext>(eglLib, "eglCreateContext");
	auto eglMakeCurrentPtr = getSymbol<EglMakeCurrent>(eglLib, "eglMakeCurrent");
	auto eglDestroySurfacePtr = getSymbol<EglDestroySurface>(eglLib, "eglDestroySurface");
	auto eglDestroyContextPtr = getSymbol<EglDestroyContext>(eglLib, "eglDestroyContext");
	auto eglTerminatePtr = getSymbol<EglTerminate>(eglLib, "eglTerminate");
	auto eglGetErrorPtr = getSymbol<EglGetError>(eglLib, "eglGetError");

	if (!eglGetProcAddressPtr || !eglGetDisplayPtr || !eglInitializePtr || !eglQueryStringPtr ||
	    !eglBindApiPtr || !eglChooseConfigPtr || !eglCreatePbufferSurfacePtr || !eglCreateContextPtr ||
	    !eglMakeCurrentPtr || !eglDestroySurfacePtr || !eglDestroyContextPtr || !eglTerminatePtr ||
	    !eglGetErrorPtr) {
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Required EGL entry points are not available");
		RET_NULL;
	}

	const char *clientExtensions = eglQueryStringPtr(nullptr, EGL_EXTENSIONS_VALUE);
	auto eglGetPlatformDisplayExtPtr =
	    reinterpret_cast<EglGetPlatformDisplayExt>(eglGetProcAddressPtr("eglGetPlatformDisplayEXT"));
	const bool canUseSurfaceless = eglGetPlatformDisplayExtPtr &&
	    hasExtension(clientExtensions, "EGL_EXT_platform_base") &&
	    hasExtension(clientExtensions, "EGL_MESA_platform_surfaceless");

	EglDisplay display = canUseSurfaceless
	    ? eglGetPlatformDisplayExtPtr(EGL_PLATFORM_SURFACELESS_MESA_VALUE, nullptr, nullptr)
	    : eglGetDisplayPtr(nullptr);

	if (!display) {
		EglInt error = eglGetErrorPtr();
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to get EGL display, error " + std::to_string(error));
		RET_NULL;
	}

	EglInt major = 0;
	EglInt minor = 0;
	if (eglInitializePtr(display, &major, &minor) == EGL_FALSE_VALUE) {
		EglInt error = eglGetErrorPtr();
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to initialize EGL display, error " + std::to_string(error));
		RET_NULL;
	}

	if (eglBindApiPtr(EGL_OPENGL_ES_API_VALUE) == EGL_FALSE_VALUE) {
		EglInt error = eglGetErrorPtr();
		eglTerminatePtr(display);
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to bind OpenGL ES API, error " + std::to_string(error));
		RET_NULL;
	}

	EglInt configAttrs[] = {
		EGL_SURFACE_TYPE_VALUE,
		EGL_PBUFFER_BIT_VALUE,
		EGL_RENDERABLE_TYPE_VALUE,
		EGL_OPENGL_ES2_BIT_VALUE,
		EGL_RED_SIZE_VALUE,
		8,
		EGL_GREEN_SIZE_VALUE,
		8,
		EGL_BLUE_SIZE_VALUE,
		8,
		EGL_ALPHA_SIZE_VALUE,
		8,
		EGL_NONE_VALUE,
	};
	EglConfig config = nullptr;
	EglInt configCount = 0;
	if (eglChooseConfigPtr(display, configAttrs, &config, 1, &configCount) == EGL_FALSE_VALUE ||
	    configCount < 1) {
		EglInt error = eglGetErrorPtr();
		eglTerminatePtr(display);
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to choose EGL config, error " + std::to_string(error));
		RET_NULL;
	}

	EglInt surfaceAttrs[] = {
		EGL_WIDTH_VALUE, static_cast<EglInt>(width), EGL_HEIGHT_VALUE, static_cast<EglInt>(height),
		EGL_NONE_VALUE,
	};
	EglSurface surface = eglCreatePbufferSurfacePtr(display, config, surfaceAttrs);
	if (!surface) {
		EglInt error = eglGetErrorPtr();
		eglTerminatePtr(display);
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to create EGL pbuffer surface, error " + std::to_string(error));
		RET_NULL;
	}

	EglInt contextAttrs[] = {
		EGL_CONTEXT_CLIENT_VERSION_VALUE,
		2,
		EGL_NONE_VALUE,
	};
	EglContext context = eglCreateContextPtr(display, config, nullptr, contextAttrs);
	if (!context) {
		EglInt error = eglGetErrorPtr();
		eglDestroySurfacePtr(display, surface);
		eglTerminatePtr(display);
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to create EGL context, error " + std::to_string(error));
		RET_NULL;
	}

	if (eglMakeCurrentPtr(display, surface, surface, context) == EGL_FALSE_VALUE) {
		EglInt error = eglGetErrorPtr();
		eglDestroyContextPtr(display, context);
		eglDestroySurfacePtr(display, surface);
		eglTerminatePtr(display);
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Unable to make EGL context current, error " + std::to_string(error));
		RET_NULL;
	}

	auto glViewportPtr = getGlSymbol<GlViewport>(glesLib, eglGetProcAddressPtr, "glViewport");
	auto glClearColorPtr = getGlSymbol<GlClearColor>(glesLib, eglGetProcAddressPtr, "glClearColor");
	auto glClearPtr = getGlSymbol<GlClear>(glesLib, eglGetProcAddressPtr, "glClear");
	auto glReadPixelsPtr = getGlSymbol<GlReadPixels>(glesLib, eglGetProcAddressPtr, "glReadPixels");
	auto glGetStringPtr = getGlSymbol<GlGetString>(glesLib, eglGetProcAddressPtr, "glGetString");

	if (!glViewportPtr || !glClearColorPtr || !glClearPtr || !glReadPixelsPtr || !glGetStringPtr) {
		eglMakeCurrentPtr(display, nullptr, nullptr, nullptr);
		eglDestroyContextPtr(display, context);
		eglDestroySurfacePtr(display, surface);
		eglTerminatePtr(display);
		dlclose(glesLib);
		dlclose(eglLib);
		JS_THROW("Required OpenGL ES entry points are not available");
		RET_NULL;
	}

	glViewportPtr(0, 0, width, height);
	glClearColorPtr(1.0f, 0.0f, 0.0f, 1.0f);
	glClearPtr(GL_COLOR_BUFFER_BIT);

	std::vector<uint8_t> pixels(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
	glReadPixelsPtr(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::Copy(env, pixels.data(), pixels.size());
	Napi::Object result = JS_OBJECT;
	result.Set("mode", "surfaceless-egl");
	result.Set("width", width);
	result.Set("height", height);
	result.Set("major", major);
	result.Set("minor", minor);
	result.Set("surfaceless", canUseSurfaceless);
	result.Set("pixels", buffer);
	result.Set("vendor", stringOrEmpty(glGetStringPtr(GL_VENDOR)));
	result.Set("renderer", stringOrEmpty(glGetStringPtr(GL_RENDERER)));
	result.Set("version", stringOrEmpty(glGetStringPtr(GL_VERSION)));
	result.Set("clientExtensions", clientExtensions ? clientExtensions : "");

	eglMakeCurrentPtr(display, nullptr, nullptr, nullptr);
	eglDestroyContextPtr(display, context);
	eglDestroySurfacePtr(display, surface);
	eglTerminatePtr(display);
	dlclose(glesLib);
	dlclose(eglLib);

	RET_VALUE(result);
#endif
}

} // namespace glfw
