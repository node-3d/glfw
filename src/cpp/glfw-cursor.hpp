#pragma once

#include "glfw-common.hpp"


namespace glfw {
DBG_EXPORT JS_METHOD(getCursorPos);
DBG_EXPORT JS_METHOD(setCursorPos);

DBG_EXPORT JS_METHOD(createCursor);
DBG_EXPORT JS_METHOD(createStandardCursor);
DBG_EXPORT JS_METHOD(destroyCursor);
DBG_EXPORT JS_METHOD(setCursor);
} // namespace glfw
