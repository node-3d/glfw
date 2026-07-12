#pragma once

#include "glfw-common.hpp"

struct GLFWwindow;

namespace glfw {
DBG_EXPORT JS_METHOD(pollEvents);
DBG_EXPORT JS_METHOD(waitEvents);
DBG_EXPORT JS_METHOD(waitEventsTimeout);
DBG_EXPORT JS_METHOD(postEmptyEvent);

DBG_EXPORT void windowPosCB(GLFWwindow *window, int xpos, int ypos);
DBG_EXPORT void windowSizeCB(GLFWwindow *window, int w, int h);
DBG_EXPORT void windowFramebufferSizeCB(GLFWwindow *window, int w, int h);
DBG_EXPORT void windowDropCB(GLFWwindow *window, int count, const char **paths);
DBG_EXPORT void windowCloseCB(GLFWwindow *window);
DBG_EXPORT void windowRefreshCB(GLFWwindow *window);
DBG_EXPORT void windowIconifyCB(GLFWwindow *window, int iconified);
DBG_EXPORT void windowFocusCB(GLFWwindow *window, int focused);
DBG_EXPORT void keyCB(GLFWwindow *window, int key, int scancode, int action, int mods);
DBG_EXPORT void charCB(GLFWwindow *window, unsigned codepoint);
DBG_EXPORT void cursorPosCB(GLFWwindow *window, double x, double y);
DBG_EXPORT void cursorEnterCB(GLFWwindow *window, int entered);
DBG_EXPORT void mouseButtonCB(GLFWwindow *window, int button, int action, int mods);
DBG_EXPORT void scrollCB(GLFWwindow *window, double xoffset, double yoffset);
DBG_EXPORT void joystickCb(int jid, int event);
} // namespace glfw
