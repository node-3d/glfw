#pragma once

#include "glfw-common.hpp"


namespace glfw {
DBG_EXPORT Napi::Object describeMonitor(Napi::Env env, GLFWmonitor *monitor, bool isPrimary);

DBG_EXPORT JS_METHOD(getMonitors);
DBG_EXPORT JS_METHOD(getPrimaryMonitor);
} // namespace glfw
