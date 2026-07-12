#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION 1
#endif

#include "glfw-common.hpp"
#include "glfw-events.hpp"
#include "glfw-window.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(testJoystick) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, width);
	REQ_UINT32_ARG(1, height);
	REQ_FLOAT_ARG(2, translateX);
	REQ_FLOAT_ARG(3, translateY);
	REQ_FLOAT_ARG(4, translateZ);
	REQ_FLOAT_ARG(5, rotateX);
	REQ_FLOAT_ARG(6, rotateY);
	REQ_FLOAT_ARG(7, rotateZ);
	REQ_FLOAT_ARG(8, angle);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	float ratio = static_cast<float>(width) / static_cast<float>(height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glRotatef(angle, rotateX, rotateY, rotateZ);
	glTranslatef(translateX, translateY, translateZ);

	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.6f, -0.4f, 0.f);
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.6f, -0.4f, 0.f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.6f, 0.f);
	glEnd();

	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(joystickPresent) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, joy);

	bool isPresent = glfwJoystickPresent(joy);

	RET_BOOL(static_cast<bool>(isPresent));
}


DBG_EXPORT JS_METHOD(getJoystickAxes) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, joy);

	int count;
	const float *axisValues = glfwGetJoystickAxes(joy, &count);
	std::string response = "";

	for (int i = 0; i < count; i++) {
		response.append(std::to_string(axisValues[i]));
		response.append(","); // Separator
	}

	RET_STR(response);
}


DBG_EXPORT JS_METHOD(getJoystickButtons) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, joy);

	int count = 0;
	const unsigned char *response = glfwGetJoystickButtons(joy, &count);

	std::string strResponse = "";
	for (int i = 0; i < count; i++) {
		strResponse.append(std::to_string(static_cast<int>(response[i])));
		strResponse.append(",");
	}

	RET_STR(strResponse.c_str());
}


DBG_EXPORT JS_METHOD(getJoystickName) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, joy);

	const char *name = glfwGetJoystickName(joy);

	if (!name) {
		RET_NULL;
	}

	RET_STR(name);
}


DBG_EXPORT JS_METHOD(getJoystickHats) {
	NAPI_ENV;
	REQ_INT32_ARG(0, jid);

	int count;
	const unsigned char *hats = glfwGetJoystickHats(jid, &count);

	Napi::Array jsHats = JS_ARRAY;

	if (!hats) {
		RET_VALUE(jsHats);
	}

	for (int i = 0; i < count; i++) {
		jsHats.Set(i, JS_NUM(hats[i]));
	}

	RET_VALUE(jsHats);
}


DBG_EXPORT JS_METHOD(getJoystickGUID) {
	NAPI_ENV;
	REQ_INT32_ARG(0, jid);

	const char *guid = glfwGetJoystickGUID(jid);

	if (!guid) {
		RET_NULL;
	}

	RET_STR(guid);
}


DBG_EXPORT JS_METHOD(joystickIsGamepad) {
	NAPI_ENV;
	REQ_INT32_ARG(0, jid);

	RET_BOOL(glfwJoystickIsGamepad(jid) == GLFW_TRUE);
}


DBG_EXPORT JS_METHOD(updateGamepadMappings) {
	NAPI_ENV;
	REQ_STR_ARG(0, mappings);
	// https://www.glfw.org/docs/latest/input_guide.html#gamepad_mapping
	RET_BOOL(glfwUpdateGamepadMappings(mappings.c_str()) == GLFW_TRUE);
}


DBG_EXPORT JS_METHOD(getGamepadName) {
	NAPI_ENV;
	REQ_INT32_ARG(0, jid);

	const char *name = glfwGetGamepadName(jid);

	if (!name) {
		RET_NULL;
	}

	RET_STR(name);
}


DBG_EXPORT JS_METHOD(getGamepadState) {
	NAPI_ENV;
	REQ_INT32_ARG(0, jid);

	GLFWgamepadstate state;
	if (glfwGetGamepadState(jid, &state) != GLFW_TRUE) {
		RET_NULL;
	}

	Napi::Object jsState = JS_OBJECT;
	Napi::Array jsButtons = JS_ARRAY;
	Napi::Array jsAxes = JS_ARRAY;

	for (int j = 0; j < 15; j++) {
		jsButtons.Set(j, JS_NUM(state.buttons[j]));
	}

	for (int j = 0; j < 6; j++) {
		jsAxes.Set(j, JS_NUM(state.axes[j]));
	}

	jsState.Set("buttons", jsButtons);
	jsState.Set("axes", jsAxes);

	RET_VALUE(jsState);
}


} // namespace glfw
