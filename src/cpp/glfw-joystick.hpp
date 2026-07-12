#pragma once

#include "glfw-common.hpp"


namespace glfw {
DBG_EXPORT JS_METHOD(joystickPresent);
DBG_EXPORT JS_METHOD(getJoystickAxes);
DBG_EXPORT JS_METHOD(getJoystickButtons);
DBG_EXPORT JS_METHOD(getJoystickName);
DBG_EXPORT JS_METHOD(getJoystickGUID);
DBG_EXPORT JS_METHOD(getJoystickHats);
DBG_EXPORT JS_METHOD(joystickIsGamepad);
DBG_EXPORT JS_METHOD(testJoystick);
DBG_EXPORT JS_METHOD(updateGamepadMappings);
DBG_EXPORT JS_METHOD(getGamepadName);
DBG_EXPORT JS_METHOD(getGamepadState);
} // namespace glfw
