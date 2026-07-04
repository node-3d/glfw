import EventEmitter from 'node:events';
import { glfw } from '@node-3d/glfw';
import type { TVulkanDevice } from '@node-3d/glfw';

const isVulkanSupported = glfw.vulkanSupported();
console.log('Vulkan is', isVulkanSupported ? 'supported.' : 'unsupported.');

let instancePtr: number | null = null;

if (isVulkanSupported) {
	const vkExt = glfw.getRequiredInstanceExtensions();
	console.log('Vulkan extensions:', vkExt);

	instancePtr = glfw.vulkanCreateInstance();
	console.log('Created instance:', instancePtr);
}

const emitterObj = new EventEmitter();
const emitter = { emit: (t: string, e: unknown) => emitterObj.emit(t, e) };

glfw.windowHint(glfw.RESIZABLE, glfw.TRUE);
glfw.windowHint(glfw.VISIBLE, glfw.TRUE);
glfw.windowHint(glfw.AUTO_ICONIFY, glfw.FALSE);
glfw.windowHint(glfw.DECORATED, glfw.TRUE);

glfw.windowHint(glfw.CLIENT_API, glfw.NO_API);
const windowPtr = glfw.createWindow(1280, 720, emitter, 'vulkan example', undefined, true);
glfw.windowHint(glfw.CLIENT_API, glfw.OPENGL_API);

let deviceInfo: TVulkanDevice | null = null;

if (instancePtr) {
	const surfacePtr = glfw.createWindowSurface(instancePtr, windowPtr);
	console.log('Created surface:', surfacePtr);

	deviceInfo = glfw.vulkanCreateDevice(instancePtr);
	console.log('Created device:', deviceInfo);
}

if (instancePtr && deviceInfo) {
	const isSupported = glfw.getPhysicalDevicePresentationSupport(
		instancePtr,
		deviceInfo.physicalDevice,
		deviceInfo.queueFamily,
	);
	console.log('Presentation supported:', isSupported);

	const vkGetDeviceProcAddrPtr = glfw.getInstanceProcAddress(instancePtr, 'vkGetDeviceProcAddr');
	console.log('Got `vkGetDeviceProcAddr`:', vkGetDeviceProcAddrPtr);
}

const draw = () => {
	glfw.pollEvents();
};

const close = () => {
	if (instancePtr && deviceInfo) {
		glfw.vulkanDestroyDevice(instancePtr, deviceInfo.device);
		console.log('Deleted device.');
	}

	// Close the window and terminate GLFW
	glfw.destroyWindow(windowPtr);

	if (instancePtr) {
		glfw.vulkanDestroyInstance(instancePtr);
		console.log('Deleted instance.');
	}

	glfw.terminate();
	process.exit(0);
};

const loopFunc = () => {
	const shouldClose = glfw.windowShouldClose(windowPtr);
	const isEscPressed = glfw.getKey(windowPtr, glfw.KEY_ESCAPE);

	if (shouldClose || isEscPressed) {
		close();
		return;
	}

	draw();
	setImmediate(loopFunc);
};
setImmediate(loopFunc);
