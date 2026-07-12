#pragma once

#include "glfw-common.hpp"


namespace glfw {
DBG_EXPORT JS_METHOD(getError);
DBG_EXPORT JS_METHOD(getFramebufferSize);
DBG_EXPORT JS_METHOD(swapBuffers);
DBG_EXPORT JS_METHOD(swapInterval);
DBG_EXPORT JS_METHOD(extensionSupported);
} // namespace glfw
