export type TWindowMode = 'windowed' | 'borderless' | 'fullscreen';

/** Opaque native pointer represented as a number on the JS side. */
export type THandle = number;

/** Opaque native GLFWwindow pointer. */
export type TWindowHandle = object & { readonly __glfwWindow: unique symbol };

export type TSize = Readonly<{ width: number; height: number }>;

export type TPos = Readonly<{ x: number; y: number }>;

export type TFrameSize = Readonly<{
	left: number;
	top: number;
	right: number;
	bottom: number;
}>;

export type TContentScale = Readonly<{
	xscale: number;
	yscale: number;
}>;

export type TVersion = Readonly<{
	major: number;
	minor: number;
	rev: number;
}>;

export type TCbVoid = () => void;

/** Browser-like rectangle. */
export type TRect = TSize & TPos & Readonly<{
	left: number;
	top: number;
	right: number;
	bottom: number;
}>;

/** Image data for icons and cursors. */
export type TImageData = TSize & Readonly<{
	data?: Buffer | Uint8Array | Uint8ClampedArray;
	buffer?: ArrayBuffer | SharedArrayBuffer;
	noflip?: boolean;
}>;

export type TMonitorMode = TSize & Readonly<{
	/** Refresh rate. */
	rate: number;
}>;

export type TMonitor = TMonitorMode & Readonly<{
	/** Whether this monitor is primary. */
	is_primary: boolean;
	/** Screen name. */
	name: string;
	/** Global screen X position. */
	pos_x: number;
	/** Global screen Y position. */
	pos_y: number;
	/** Screen width in millimeters. */
	width_mm: number;
	/** Screen height in millimeters. */
	height_mm: number;
	/** Supported display modes. */
	modes: readonly TMonitorMode[];
}>;

export type TNativeEmitter = Readonly<{
	emit: (type: string, event: unknown) => void;
	[name: string]: unknown;
}>;

export type TGamepadState = Readonly<{
	buttons: readonly number[];
	axes: readonly number[];
}>;

export type TVulkanDevice = Readonly<{
	device: THandle;
	physicalDevice: THandle;
	queueFamily: number;
}>;

/** Event object emitted by `Window`. */
export type TEvent = {
	type: string;
	[key: string]: unknown;
};

export type TMouseEvent = TEvent & {
	buttons: number;
	clientX: number;
	clientY: number;
	pageX: number;
	pageY: number;
	x: number;
	y: number;
	shiftKey: boolean;
	ctrlKey: boolean;
	altKey: boolean;
	metaKey: boolean;
};

export type TMouseMoveEvent = TMouseEvent & {
	movementX: number;
	movementY: number;
};

export type TMouseButtonEvent = TMouseEvent & {
	button: number;
	which: number;
};

export type TMouseScrollEvent = TMouseEvent & {
	deltaX: number;
	deltaY: number;
	deltaZ: number;
	wheelDeltaX: number;
	wheelDeltaY: number;
	wheelDelta: number;
};

export type TJoystickEvent = TEvent & {
	id: number;
	event: number;
};

export type TKeyEvent = TEvent & {
	repeat: boolean;
	altKey: boolean;
	ctrlKey: boolean;
	metaKey: boolean;
	shiftKey: boolean;
	code: string | null;
	key: string | null;
	which: number;
	charCode: number;
};

export type TDropEvent = TEvent & {
	dataTransfer: Readonly<Record<string, never>>;
	dropEffect: 'none';
	effectAllowed: 'all';
	files: readonly string[];
	items: readonly string[];
	types: readonly never[];
};

export type TIconifyEvent = TEvent & {
	iconified: boolean;
};

export type TPosEvent = TEvent & TPos;

export type TSizeEvent = TEvent & TSize;

export type TEventCb<T extends TEvent> = (event: T) => (undefined | boolean);

export type TCbField<T extends TEvent> = TEventCb<T> | readonly TEventCb<T>[];

export type TWebgl = Readonly<Record<string, unknown>>;
