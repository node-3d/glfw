#pragma once

#include "glfw-common.hpp"


namespace glfw {
DBG_EXPORT void deinit();

DBG_EXPORT JS_METHOD(init);
DBG_EXPORT JS_METHOD(initHint);
DBG_EXPORT JS_METHOD(terminate);
DBG_EXPORT JS_METHOD(testScene);
} // namespace glfw
