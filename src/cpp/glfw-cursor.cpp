#include "glfw-common.hpp"
#include "glfw-cursor.hpp"


namespace glfw {

DBG_EXPORT JS_METHOD(getCursorPos) { NAPI_ENV; THIS_WINDOW;
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	
	Napi::Object obj = JS_OBJECT;
	obj.Set("x", JS_NUM(x));
	obj.Set("y", JS_NUM(y));
	
	RET_VALUE(obj);
}


DBG_EXPORT JS_METHOD(setCursorPos) { NAPI_ENV; THIS_WINDOW;
	REQ_INT32_ARG(1, x);
	REQ_INT32_ARG(2, y);
	
	glfwSetCursorPos(window, x, y);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(createCursor) { NAPI_ENV;
	REQ_OBJ_ARG(0, icon);
	
	if (!(icon.Has("width") && icon.Has("height"))) {
		RET_GLFW_VOID;
	}
	
	bool noflip = icon.Has("noflip") ? icon.Get("noflip").ToBoolean().Value() : false;
	
	GLFWimage image;
	image.width = icon.Get("width").ToNumber().Int32Value();
	image.height = icon.Get("height").ToNumber().Int32Value();
	
	int32_t numPixels = 0;
	uint8_t *src = getData<uint8_t>(env, icon, &numPixels);
	const uint64_t requiredBytes = (
		static_cast<uint64_t>(image.width) * static_cast<uint64_t>(image.height) * 4
	);
	
	if (
		image.width <= 0 ||
		image.height <= 0 ||
		src == nullptr ||
		static_cast<uint64_t>(numPixels) < requiredBytes
	) {
		JS_THROW("Cursor data must contain width * height * 4 bytes.");
		RET_UNDEFINED;
	}
	
	GLFWcursor *cursor = nullptr;
	
	if (!noflip) {
		std::vector<uint8_t> dest(static_cast<size_t>(requiredBytes));
		int32_t lastY = image.height - 1;
		for (int32_t y = 0; y < image.height; y++) {
			for (int32_t x = 0; x < image.width; x++) {
				int32_t iForward = (y * image.width + x) << 2;
				int32_t iBackward = ((lastY - y) * image.width + x) << 2;
				dest[static_cast<size_t>(iForward + 0)] = src[iBackward + 0];
				dest[static_cast<size_t>(iForward + 1)] = src[iBackward + 1];
				dest[static_cast<size_t>(iForward + 2)] = src[iBackward + 2];
				dest[static_cast<size_t>(iForward + 3)] = src[iBackward + 3];
			}
		}
		image.pixels = dest.data();
		cursor = glfwCreateCursor(&image, image.width / 2, image.height / 2);
	} else {
		image.pixels = src;
		cursor = glfwCreateCursor(&image, image.width / 2, image.height / 2);
	}
	
	RET_PTR(cursor);
}

DBG_EXPORT JS_METHOD(createStandardCursor) { NAPI_ENV;
	REQ_INT32_ARG(0, shape);
	GLFWcursor *cursor = glfwCreateStandardCursor(shape);
	RET_PTR(cursor);
}


DBG_EXPORT JS_METHOD(destroyCursor) { NAPI_ENV;
	REQ_OFFS_ARG(0, cursorPtr);
	GLFWcursor *cursor = reinterpret_cast<GLFWcursor*>(cursorPtr);
	glfwDestroyCursor(cursor);
	RET_GLFW_VOID;
}


DBG_EXPORT JS_METHOD(setCursor) { NAPI_ENV; THIS_WINDOW;
	REQ_OFFS_ARG(1, cursorPtr);
	GLFWcursor *cursor = reinterpret_cast<GLFWcursor*>(cursorPtr);
	glfwSetCursor(window, cursor);
	RET_GLFW_VOID;
}

} // namespace glfw
