import { createRequire } from 'node:module';
import { getBin } from '@node-3d/addon-tools';
import '@node-3d/segfault';
import '@node-3d/deps-opengl';
import type {
	TContentScale,
	TFrameSize,
	TGamepadState,
	THandle,
	TImageData,
	TMonitor,
	TNativeEmitter,
	TPos,
	TSize,
	TVersion,
	TVulkanDevice,
	TWindowHandle,
} from './types.ts';

type TGlfwConstant =
	| 'VERSION_MAJOR'
	| 'VERSION_MINOR'
	| 'VERSION_REVISION'
	| 'TRUE'
	| 'FALSE'
	| 'RELEASE'
	| 'PRESS'
	| 'REPEAT'
	| 'HAT_CENTERED'
	| 'HAT_UP'
	| 'HAT_RIGHT'
	| 'HAT_DOWN'
	| 'HAT_LEFT'
	| 'HAT_RIGHT_UP'
	| 'HAT_RIGHT_DOWN'
	| 'HAT_LEFT_UP'
	| 'HAT_LEFT_DOWN'
	| 'KEY_UNKNOWN'
	| 'KEY_SPACE'
	| 'KEY_APOSTROPHE'
	| 'KEY_COMMA'
	| 'KEY_MINUS'
	| 'KEY_PERIOD'
	| 'KEY_SLASH'
	| 'KEY_0'
	| 'KEY_1'
	| 'KEY_2'
	| 'KEY_3'
	| 'KEY_4'
	| 'KEY_5'
	| 'KEY_6'
	| 'KEY_7'
	| 'KEY_8'
	| 'KEY_9'
	| 'KEY_SEMICOLON'
	| 'KEY_EQUAL'
	| 'KEY_A'
	| 'KEY_B'
	| 'KEY_C'
	| 'KEY_D'
	| 'KEY_E'
	| 'KEY_F'
	| 'KEY_G'
	| 'KEY_H'
	| 'KEY_I'
	| 'KEY_J'
	| 'KEY_K'
	| 'KEY_L'
	| 'KEY_M'
	| 'KEY_N'
	| 'KEY_O'
	| 'KEY_P'
	| 'KEY_Q'
	| 'KEY_R'
	| 'KEY_S'
	| 'KEY_T'
	| 'KEY_U'
	| 'KEY_V'
	| 'KEY_W'
	| 'KEY_X'
	| 'KEY_Y'
	| 'KEY_Z'
	| 'KEY_LEFT_BRACKET'
	| 'KEY_BACKSLASH'
	| 'KEY_RIGHT_BRACKET'
	| 'KEY_GRAVE_ACCENT'
	| 'KEY_WORLD_1'
	| 'KEY_WORLD_2'
	| 'KEY_ESCAPE'
	| 'KEY_ENTER'
	| 'KEY_TAB'
	| 'KEY_BACKSPACE'
	| 'KEY_INSERT'
	| 'KEY_DELETE'
	| 'KEY_RIGHT'
	| 'KEY_LEFT'
	| 'KEY_DOWN'
	| 'KEY_UP'
	| 'KEY_PAGE_UP'
	| 'KEY_PAGE_DOWN'
	| 'KEY_HOME'
	| 'KEY_END'
	| 'KEY_CAPS_LOCK'
	| 'KEY_SCROLL_LOCK'
	| 'KEY_NUM_LOCK'
	| 'KEY_PRINT_SCREEN'
	| 'KEY_PAUSE'
	| 'KEY_F1'
	| 'KEY_F2'
	| 'KEY_F3'
	| 'KEY_F4'
	| 'KEY_F5'
	| 'KEY_F6'
	| 'KEY_F7'
	| 'KEY_F8'
	| 'KEY_F9'
	| 'KEY_F10'
	| 'KEY_F11'
	| 'KEY_F12'
	| 'KEY_F13'
	| 'KEY_F14'
	| 'KEY_F15'
	| 'KEY_F16'
	| 'KEY_F17'
	| 'KEY_F18'
	| 'KEY_F19'
	| 'KEY_F20'
	| 'KEY_F21'
	| 'KEY_F22'
	| 'KEY_F23'
	| 'KEY_F24'
	| 'KEY_F25'
	| 'KEY_KP_0'
	| 'KEY_KP_1'
	| 'KEY_KP_2'
	| 'KEY_KP_3'
	| 'KEY_KP_4'
	| 'KEY_KP_5'
	| 'KEY_KP_6'
	| 'KEY_KP_7'
	| 'KEY_KP_8'
	| 'KEY_KP_9'
	| 'KEY_KP_DECIMAL'
	| 'KEY_KP_DIVIDE'
	| 'KEY_KP_MULTIPLY'
	| 'KEY_KP_SUBTRACT'
	| 'KEY_KP_ADD'
	| 'KEY_KP_ENTER'
	| 'KEY_KP_EQUAL'
	| 'KEY_LEFT_SHIFT'
	| 'KEY_LEFT_CONTROL'
	| 'KEY_LEFT_ALT'
	| 'KEY_LEFT_SUPER'
	| 'KEY_RIGHT_SHIFT'
	| 'KEY_RIGHT_CONTROL'
	| 'KEY_RIGHT_ALT'
	| 'KEY_RIGHT_SUPER'
	| 'KEY_MENU'
	| 'KEY_LAST'
	| 'MOD_SHIFT'
	| 'MOD_CONTROL'
	| 'MOD_ALT'
	| 'MOD_SUPER'
	| 'MOD_CAPS_LOCK'
	| 'MOD_NUM_LOCK'
	| 'MOUSE_BUTTON_1'
	| 'MOUSE_BUTTON_2'
	| 'MOUSE_BUTTON_3'
	| 'MOUSE_BUTTON_4'
	| 'MOUSE_BUTTON_5'
	| 'MOUSE_BUTTON_6'
	| 'MOUSE_BUTTON_7'
	| 'MOUSE_BUTTON_8'
	| 'MOUSE_BUTTON_LAST'
	| 'MOUSE_BUTTON_LEFT'
	| 'MOUSE_BUTTON_RIGHT'
	| 'MOUSE_BUTTON_MIDDLE'
	| 'JOYSTICK_1'
	| 'JOYSTICK_2'
	| 'JOYSTICK_3'
	| 'JOYSTICK_4'
	| 'JOYSTICK_5'
	| 'JOYSTICK_6'
	| 'JOYSTICK_7'
	| 'JOYSTICK_8'
	| 'JOYSTICK_9'
	| 'JOYSTICK_10'
	| 'JOYSTICK_11'
	| 'JOYSTICK_12'
	| 'JOYSTICK_13'
	| 'JOYSTICK_14'
	| 'JOYSTICK_15'
	| 'JOYSTICK_16'
	| 'JOYSTICK_LAST'
	| 'GAMEPAD_BUTTON_A'
	| 'GAMEPAD_BUTTON_B'
	| 'GAMEPAD_BUTTON_X'
	| 'GAMEPAD_BUTTON_Y'
	| 'GAMEPAD_BUTTON_LEFT_BUMPER'
	| 'GAMEPAD_BUTTON_RIGHT_BUMPER'
	| 'GAMEPAD_BUTTON_BACK'
	| 'GAMEPAD_BUTTON_START'
	| 'GAMEPAD_BUTTON_GUIDE'
	| 'GAMEPAD_BUTTON_LEFT_THUMB'
	| 'GAMEPAD_BUTTON_RIGHT_THUMB'
	| 'GAMEPAD_BUTTON_DPAD_UP'
	| 'GAMEPAD_BUTTON_DPAD_RIGHT'
	| 'GAMEPAD_BUTTON_DPAD_DOWN'
	| 'GAMEPAD_BUTTON_DPAD_LEFT'
	| 'GAMEPAD_BUTTON_LAST'
	| 'GAMEPAD_BUTTON_CROSS'
	| 'GAMEPAD_BUTTON_CIRCLE'
	| 'GAMEPAD_BUTTON_SQUARE'
	| 'GAMEPAD_BUTTON_TRIANGLE'
	| 'GAMEPAD_AXIS_LEFT_X'
	| 'GAMEPAD_AXIS_LEFT_Y'
	| 'GAMEPAD_AXIS_RIGHT_X'
	| 'GAMEPAD_AXIS_RIGHT_Y'
	| 'GAMEPAD_AXIS_LEFT_TRIGGER'
	| 'GAMEPAD_AXIS_RIGHT_TRIGGER'
	| 'GAMEPAD_AXIS_LAST'
	| 'NO_ERROR'
	| 'NOT_INITIALIZED'
	| 'NO_CURRENT_CONTEXT'
	| 'INVALID_ENUM'
	| 'INVALID_VALUE'
	| 'OUT_OF_MEMORY'
	| 'API_UNAVAILABLE'
	| 'VERSION_UNAVAILABLE'
	| 'PLATFORM_ERROR'
	| 'FORMAT_UNAVAILABLE'
	| 'NO_WINDOW_CONTEXT'
	| 'CURSOR_UNAVAILABLE'
	| 'FEATURE_UNAVAILABLE'
	| 'FEATURE_UNIMPLEMENTED'
	| 'PLATFORM_UNAVAILABLE'
	| 'FOCUSED'
	| 'ICONIFIED'
	| 'RESIZABLE'
	| 'VISIBLE'
	| 'DECORATED'
	| 'AUTO_ICONIFY'
	| 'FLOATING'
	| 'MAXIMIZED'
	| 'CENTER_CURSOR'
	| 'TRANSPARENT_FRAMEBUFFER'
	| 'HOVERED'
	| 'FOCUS_ON_SHOW'
	| 'MOUSE_PASSTHROUGH'
	| 'POSITION_X'
	| 'POSITION_Y'
	| 'RED_BITS'
	| 'GREEN_BITS'
	| 'BLUE_BITS'
	| 'ALPHA_BITS'
	| 'DEPTH_BITS'
	| 'STENCIL_BITS'
	| 'ACCUM_RED_BITS'
	| 'ACCUM_GREEN_BITS'
	| 'ACCUM_BLUE_BITS'
	| 'ACCUM_ALPHA_BITS'
	| 'AUX_BUFFERS'
	| 'STEREO'
	| 'SAMPLES'
	| 'SRGB_CAPABLE'
	| 'REFRESH_RATE'
	| 'DOUBLEBUFFER'
	| 'CLIENT_API'
	| 'CONTEXT_VERSION_MAJOR'
	| 'CONTEXT_VERSION_MINOR'
	| 'CONTEXT_REVISION'
	| 'CONTEXT_ROBUSTNESS'
	| 'OPENGL_FORWARD_COMPAT'
	| 'CONTEXT_DEBUG'
	| 'OPENGL_DEBUG_CONTEXT'
	| 'OPENGL_PROFILE'
	| 'CONTEXT_RELEASE_BEHAVIOR'
	| 'CONTEXT_NO_ERROR'
	| 'CONTEXT_CREATION_API'
	| 'SCALE_TO_MONITOR'
	| 'SCALE_FRAMEBUFFER'
	| 'COCOA_RETINA_FRAMEBUFFER'
	| 'COCOA_FRAME_NAME'
	| 'COCOA_GRAPHICS_SWITCHING'
	| 'X11_CLASS_NAME'
	| 'X11_INSTANCE_NAME'
	| 'NO_API'
	| 'OPENGL_API'
	| 'OPENGL_ES_API'
	| 'NO_ROBUSTNESS'
	| 'NO_RESET_NOTIFICATION'
	| 'LOSE_CONTEXT_ON_RESET'
	| 'OPENGL_ANY_PROFILE'
	| 'OPENGL_CORE_PROFILE'
	| 'OPENGL_COMPAT_PROFILE'
	| 'CURSOR'
	| 'STICKY_KEYS'
	| 'STICKY_MOUSE_BUTTONS'
	| 'LOCK_KEY_MODS'
	| 'RAW_MOUSE_MOTION'
	| 'CURSOR_NORMAL'
	| 'CURSOR_HIDDEN'
	| 'CURSOR_DISABLED'
	| 'CURSOR_CAPTURED'
	| 'ARROW_CURSOR'
	| 'IBEAM_CURSOR'
	| 'CROSSHAIR_CURSOR'
	| 'HAND_CURSOR'
	| 'HRESIZE_CURSOR'
	| 'VRESIZE_CURSOR'
	| 'POINTING_HAND_CURSOR'
	| 'RESIZE_EW_CURSOR'
	| 'RESIZE_NS_CURSOR'
	| 'RESIZE_NWSE_CURSOR'
	| 'RESIZE_NESW_CURSOR'
	| 'RESIZE_ALL_CURSOR'
	| 'NOT_ALLOWED_CURSOR'
	| 'ANY_RELEASE_BEHAVIOR'
	| 'RELEASE_BEHAVIOR_FLUSH'
	| 'RELEASE_BEHAVIOR_NONE'
	| 'NATIVE_CONTEXT_API'
	| 'EGL_CONTEXT_API'
	| 'OSMESA_CONTEXT_API'
	| 'WIN32_KEYBOARD_MENU'
	| 'WIN32_SHOWDEFAULT'
	| 'WAYLAND_APP_ID'
	| 'ANGLE_PLATFORM_TYPE_NONE'
	| 'ANGLE_PLATFORM_TYPE_OPENGL'
	| 'ANGLE_PLATFORM_TYPE_OPENGLES'
	| 'ANGLE_PLATFORM_TYPE_D3D9'
	| 'ANGLE_PLATFORM_TYPE_D3D11'
	| 'ANGLE_PLATFORM_TYPE_VULKAN'
	| 'ANGLE_PLATFORM_TYPE_METAL'
	| 'WAYLAND_PREFER_LIBDECOR'
	| 'WAYLAND_DISABLE_LIBDECOR'
	| 'ANGLE_PLATFORM_TYPE'
	| 'PLATFORM'
	| 'X11_XCB_VULKAN_SURFACE'
	| 'WAYLAND_LIBDECOR'
	| 'ANY_PLATFORM'
	| 'PLATFORM_WIN32'
	| 'PLATFORM_COCOA'
	| 'PLATFORM_WAYLAND'
	| 'PLATFORM_X11'
	| 'PLATFORM_NULL'
	| 'CONNECTED'
	| 'DISCONNECTED'
	| 'JOYSTICK_HAT_BUTTONS'
	| 'COCOA_CHDIR_RESOURCES'
	| 'COCOA_MENUBAR'
	| 'DONT_CARE'
	| 'ANY_POSITION';

type TNativeMethods = {
	/**
	 * Hide the terminal window.
	 *
	 * Windows only. Hides the console window if this process owns it. This is
	 * safe to call on all platforms, but ignored unless the platform is Windows.
	 */
	hideConsole(): void;
	/**
	 * Show the terminal window.
	 *
	 * Windows only. Shows the console window if it was previously hidden with
	 * `glfw.hideConsole()`.
	 */
	showConsole(): void;
	init(): boolean;
	initHint(hint: number, value: number): void;
	terminate(): void;
	getVersion(): TVersion;
	getVersionString(): string;
	getError(): string | null;
	getTime(): number;
	setTime(time: number): void;
	getMonitors(): readonly TMonitor[];
	getPrimaryMonitor(): TMonitor | null;
	windowHint(hint: number, value: number): void;
	windowHintString(hint: number, value: string): void;
	defaultWindowHints(): void;
	joystickPresent(joy: number): boolean;
	getJoystickAxes(joy: number): string;
	getJoystickButtons(joy: number): string;
	getJoystickName(joy: number): string | null;
	/**
	 * Create a GLFW window.
	 *
	 * This differs from the GLFW C signature due to JS specifics. The `emitter`
	 * object must have a bound `emit()` method used to transmit GLFW events.
	 */
	createWindow(
		width: number,
		height: number,
		emitter: TNativeEmitter,
		title?: string,
		monitorIndex?: number,
		isNoApi?: boolean,
	): TWindowHandle;
	destroyWindow(window: TWindowHandle): void;
	setWindowTitle(window: TWindowHandle, title: string): void;
	setWindowIcon(window: TWindowHandle, icon: TImageData): void;
	getWindowSize(window: TWindowHandle): TSize;
	getWindowFrameSize(window: TWindowHandle): TFrameSize;
	getWindowContentScale(window: TWindowHandle): TContentScale;
	setWindowSize(window: TWindowHandle, width: number, height: number): void;
	setWindowSizeLimits(
		window: TWindowHandle,
		minWidth: number,
		minHeight: number,
		maxWidth: number,
		maxHeight: number,
	): void;
	setWindowAspectRatio(window: TWindowHandle, numerator?: number, denominator?: number): void;
	setWindowPos(window: TWindowHandle, x: number, y: number): void;
	getWindowPos(window: TWindowHandle): TPos;
	getWindowOpacity(window: TWindowHandle): number;
	setWindowOpacity(window: TWindowHandle, opacity: number): void;
	maximizeWindow(window: TWindowHandle): void;
	focusWindow(window: TWindowHandle): void;
	requestWindowAttention(window: TWindowHandle): void;
	getWindowMonitor(window: TWindowHandle): TMonitor | null;
	getFramebufferSize(window: TWindowHandle): TSize;
	iconifyWindow(window: TWindowHandle): void;
	restoreWindow(window: TWindowHandle): void;
	hideWindow(window: TWindowHandle): void;
	showWindow(window: TWindowHandle): void;
	windowShouldClose(window: TWindowHandle): number;
	setWindowShouldClose(window: TWindowHandle, value: number): void;
	getWindowAttrib(window: TWindowHandle, attrib: number): number;
	setWindowAttrib(window: TWindowHandle, attrib: number, value: number): void;
	/**
	 * Draw one frame by polling events, calling `callback`, and swapping buffers.
	 */
	drawWindow(window: TWindowHandle, callback: (time: number) => void): void;
	setInputMode(window: TWindowHandle, mode: number, value: number): void;
	getInputMode(window: TWindowHandle, mode: number): number;
	pollEvents(): void;
	waitEvents(): void;
	waitEventsTimeout(timeout: number): void;
	postEmptyEvent(): void;
	getKey(window: TWindowHandle, key: number): number;
	getMouseButton(window: TWindowHandle, button: number): number;
	getCursorPos(window: TWindowHandle): TPos;
	setCursorPos(window: TWindowHandle, x: number, y: number): void;
	makeContextCurrent(window: TWindowHandle): void;
	getCurrentContext(window?: TWindowHandle): TWindowHandle | null;
	swapBuffers(window: TWindowHandle): void;
	swapInterval(interval: number): void;
	extensionSupported(extension: string): boolean;
	rawMouseMotionSupported(): boolean;
	getKeyName(key: number, scancode: number): string | null;
	getKeyScancode(key: number): number;
	createCursor(icon: TImageData): THandle | undefined;
	createStandardCursor(shape: number): THandle;
	destroyCursor(cursor: THandle): void;
	setCursor(window: TWindowHandle, cursor: THandle): void;
	getJoystickHats(jid: number): readonly number[];
	joystickIsGamepad(jid: number): boolean;
	updateGamepadMappings(mappings: string): boolean;
	getGamepadName(jid: number): string | null;
	getGamepadState(jid: number): TGamepadState | null;
	setClipboardString(window: TWindowHandle, value: string): void;
	getClipboardString(window: TWindowHandle): string;
	getTimerValue(): number;
	getTimerFrequency(): number;
	platformDevice(): THandle;
	platformWindow(window: TWindowHandle): THandle;
	platformContext(window: TWindowHandle): THandle;
	getPlatform(): number;
	platformSupported(platform: number): number;
	/** Draws a test scene, used by examples in this package. */
	testScene(width?: number, height?: number, z?: number): void;
	/** Draws a test scene that reacts to a joystick. */
	testJoystick(
		width: number,
		height: number,
		translateX: number,
		translateY: number,
		translateZ: number,
		rotateX: number,
		rotateY: number,
		rotateZ: number,
		angle: number,
	): void;
	getJoystickGUID(jid: number): string | null;
	vulkanSupported(): boolean;
	getRequiredInstanceExtensions(): readonly string[];
	getInstanceProcAddress(instance: THandle, name: string): THandle;
	getPhysicalDevicePresentationSupport(
		instance: THandle,
		physicalDevice: THandle,
		queueFamily: number,
	): boolean;
	createWindowSurface(
		instance: THandle,
		window: TWindowHandle,
		allocator?: THandle,
	): THandle | null;
	vulkanCreateInstance(): THandle | null;
	vulkanCreateDevice(instance: THandle): TVulkanDevice | null;
	vulkanDestroyDevice(instance: THandle, device: THandle): void;
	vulkanDestroyInstance(instance: THandle): void;
};

type TNative = TNativeMethods & Record<TGlfwConstant, number>;

const loadAddon = createRequire(import.meta.url);
const loaded = loadAddon(`../${getBin()}/glfw.node`);

/**
 * Raw GLFW native bindings.
 *
 * Most `glfw*` functions are exposed without the `glfw` prefix, e.g.
 * `glfwPollEvents` becomes `glfw.pollEvents`. GLFW constants are exposed as
 * numeric properties without the `GLFW_` prefix, e.g. `GLFW_TRUE` becomes
 * `glfw.TRUE`.
 */
export const native = loaded as TNative;
