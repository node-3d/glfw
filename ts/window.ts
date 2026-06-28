import {
	emptyFunction, keyNames, codeNames, extraCodes,
} from './constants.ts';
import EventEmitter from 'node:events';
import { glfw } from './core.ts';
import type {
	TCbField,
	TEvent,
	TEventCb,
	TImageData,
	TKeyEvent,
	TMonitor,
	TMonitorMode,
	TMouseButtonEvent,
	TMouseScrollEvent,
	TPos,
	TRect,
	TSize,
	TNativeEmitter,
	TWindowHandle,
	TWindowMode,
} from './types.ts';

// oxlint-disable max-lines

const DEFAULT_WIDTH = 1280;
const DEFAULT_HEIGHT = 720;

export type TWindowOpts = Readonly<Partial<{
	/** Major OpenGL version to be used. Default is 2. */
	major: number;
	/** Minor OpenGL version to be used. Default is 1. */
	minor: number;
	/** Initial logical window width. Default is 1280. */
	width: number;
	/** Initial logical window height. Default is 720. */
	height: number;
	/** Display id to open the window on. Default is 0. */
	display: number;
	/** Whether vsync should be used. Default is false. */
	vsync: boolean;
	/** Whether fullscreen windows should iconify automatically on focus loss. Default is true. */
	autoIconify: boolean;
	/** Window display mode. Default is 'windowed'. */
	mode: TWindowMode;
	/** Whether the window has borders. Default is true. */
	decorated: boolean;
	/** Multisample antialiasing level. Default is 0. */
	msaa: number;
	/** Window icon. Default is null. */
	icon: TImageData;
	/** Window title. Default is current working directory. */
	title: string;
	/** Whether the window is resizable. Default is true. */
	resizable: boolean;
	/**
	 * Called right before window creation.
	 *
	 * `window` is the current Window object. `glfw` is the raw GLFW binding.
	 */
	onBeforeWindow: (window: Window, glfw: unknown) => void;
}>>;

/**
 * GLFW Window API wrapper.
 *
 * Window is a higher-level JS wrapper for the GLFW API. It helps managing
 * window instances and extends EventEmitter to provide event handling.
 */
export class Window extends EventEmitter {
	public constructor(opts: TWindowOpts = {}) {
		super();
		
		this._readOptions(opts);
		this._applyWindowHints();
		this._emitter = { emit: (t, e) => this.emit(t, e as TEvent) };
		
		// This CREATES window, as mode switches from `undefined`
		this.mode = opts.mode || 'windowed';
		
		this._syncFramebufferRatio();
		this._applyInitialAppearance(opts);
		this._readContextAttributes();
		this._bindStateEvents();
		
		this.requestAnimationFrame = (cb) => setImmediate(() => glfw.drawWindow(this._window, cb));
		this.cancelAnimationFrame = (id) => { clearImmediate(id); };
		
		this._rafCb = () => { /* nop */ };
		this._drawWithCb = () => glfw.drawWindow(this._window, this._rafCb);
		
		this.frame = (cb) => {
			this._rafCb = cb;
			return setImmediate(this._drawWithCb);
		};
		
		this.loop = (cb) => {
			let next: NodeJS.Immediate | null = null;
			const loopFunc = () => {
				glfw.drawWindow(this._window, cb);
				next = setImmediate(loopFunc);
			};
			next = setImmediate(loopFunc);
			return () => {
				if (next) {
					clearImmediate(next);
				}
			};
		};
		
		this.event = null;
		
		this.swapBuffers();
	}
	
	/** The ratio between physical and logical pixels, e.g. 2 for Retina. Default is 1. */
	public get ratio(): number { return this._ratio; }
	
	/** Alias for `ratio`. */
	public get devicePixelRatio(): number { return this._ratio; }
	
	
	/** GLFW window pointer, represented as a number. */
	public get handle(): TWindowHandle { return this._window; }
	
	/** Always 0. */
	public get scrollX(): number { return 0; }
	
	/** Always 0. */
	public get scrollY(): number { return 0; }
	
	/** Number of MSAA samples. */
	public get msaa(): number { return this._msaa; }
	
	/** OpenGL vendor/version info. */
	public get version(): string {
		return `GL ${this._major}.${this._minor}.${this._rev} Profile: ${this._prof}`;
	}
	
	/** Platform device, e.g. `wglGetCurrentDC()` or `glfwGetX11Display()`. */
	public get platformDevice(): number { return glfw.platformDevice(); }
	
	/** Platform-specific window handle, e.g. Windows HWND. */
	public get platformWindow(): number { return glfw.platformWindow(this._window); }
	
	/** Platform OpenGL context id for this window. */
	public get platformContext(): number { return glfw.platformContext(this._window); }
	
	/** The size of the allocated framebuffer. */
	public get framebufferSize(): TSize { return glfw.getFramebufferSize(this._window); }
	
	/** Current OpenGL context pointer. */
	public get currentContext(): TWindowHandle | null { return glfw.getCurrentContext(this._window); }
	
	/**
	 * Window display mode.
	 *
	 * One of: 'windowed', 'borderless', 'fullscreen'. Here 'borderless' emulates
	 * fullscreen by a frameless, screen-sized window. When this property changes,
	 * a new window is created and the old one is hidden.
	 */
	public get mode(): TWindowMode { return this._mode ?? 'windowed'; }
	
	public set mode(v: TWindowMode) {
		if (this._mode === v) {
			return;
		}
		
		const currentMonitor = this.getCurrentMonitor();
		
		const prevMode = this._mode;
		this._mode = v;
		
		if (this._window) {
			// Fullscreen can't be hidden (uh-oh)
			if (prevMode === 'fullscreen') {
				this.destroy();
				this._modeCache[prevMode] = null;
			} else {
				this.hide();
			}
		}
		
		if (this._monitors.length === 0) {
			throw new Error('No suitable display found for a new GLFW Window.');
		}
		
		this._display = currentMonitor ? this._monitors.indexOf(currentMonitor) : 0;
		
		if (this._mode === 'windowed') {
			this._x = this._prevX || this._x;
			this._y = this._prevY || this._y;
			this._width = this._prevWidth || this._width;
			this._height = this._prevHeight || this._height;
			this._decorated = this._prevDecorated || this._decorated;
			this._prevX = null;
			this._prevY = null;
			this._prevWidth = null;
			this._prevHeight = null;
			this._prevDecorated = null;
		} else if (
			currentMonitor && (
				this._width !== currentMonitor.width ||
				this._height !== currentMonitor.height
			)
		) {
			this._prevX = this._x;
			this._prevY = this._y;
			this._prevWidth = this._width;
			this._prevHeight = this._height;
			this._x = currentMonitor.pos_x;
			this._y = currentMonitor.pos_y;
			this._width = currentMonitor.width;
			this._height = currentMonitor.height;
		}
		
		const prevWindow = this._modeCache[this._mode];
		if (prevWindow) {
			this._window = prevWindow;
			this.show();
		} else {
			this._create();
			this._modeCache[this._mode] = this._window;
		}
		
		if (this._mode === 'windowed') {
			if (this._x && this._y) {
				glfw.setWindowPos(this._window, this._x, this._y);
			}
		} else if (this._mode === 'borderless') {
			const monitor = this._monitors[this._display];
			glfw.setWindowPos(this._window, monitor.pos_x, monitor.pos_y);
			glfw.setWindowSize(this._window, monitor.width, monitor.height);
		}
		
		this.makeCurrent();
		glfw.swapInterval(this._vsync);
		
		this._pxWidth = this._width * this._ratio;
		this._pxHeight = this._height * this._ratio;
		
		this.emit('mode', { type: 'mode', mode: this._mode });
		this.emit('resize', { type: 'resize', width: this._pxWidth, height: this._pxHeight });
	}
	
	/** Width in physical pixels. */
	public get width(): number { return this._pxWidth; }
	public set width(v: number) {
		if (this._pxWidth === v) {
			return;
		}
		this._width = Math.floor(v / this._ratio);
		this._pxWidth = v;
		
		glfw.setWindowSize(this._window, this._width, this._height);
	}
	
	/** Height in physical pixels. */
	public get height(): number { return this._pxHeight; }
	public set height(v: number) {
		if (this._pxHeight === v) {
			return;
		}
		this._height = Math.floor(v / this._ratio);
		this._pxHeight = v;
		glfw.setWindowSize(this._window, this._width, this._height);
	}
	
	/** Alias for `width`. */
	public get offsetWidth(): number { return this._pxWidth; }
	public set offsetWidth(v: number) { this.width = v; }
	/** Alias for `height`. */
	public get offsetHeight(): number { return this._pxHeight; }
	public set offsetHeight(v: number) { this.height = v; }
	
	/** Alias for `width`. */
	public get w(): number { return this.width; }
	public set w(v: number) { this.width = v; }
	/** Alias for `height`. */
	public get h(): number { return this.height; }
	public set h(v: number) { this.height = v; }
	/** Physical width and height as a tuple. */
	public get wh(): [number, number]{ return [this.width, this.height]; }
	public set wh([width, height]: [number, number]) {
		if (this._pxWidth === width && this._pxHeight === height) {
			return;
		}
		this._width = Math.floor(width / this._ratio);
		this._pxWidth = width;
		this._height = Math.floor(height / this._ratio);
		this._pxHeight = height;
		glfw.setWindowSize(this._window, this._width, this._height);
	}
	/** The size of the allocated framebuffer. */
	public get pxSize(): TSize {
		const size = glfw.getFramebufferSize(this._window);
		this._pxWidth = size.width;
		this._pxHeight = size.height;
		this._width = this._pxWidth / this._ratio;
		this._height = this._pxHeight / this._ratio;
		return size;
	}
	
	public set pxSize({ width, height }: TSize) {
		this.wh = [width, height];
	}
	
	/** Logical window width. */
	public get innerWidth(): number { return this._width; }
	public set innerWidth(v: number) { this.width = v; }
	/** Logical window height. */
	public get innerHeight(): number { return this._height; }
	public set innerHeight(v: number) { this.height = v; }
	
	/** Alias for `innerWidth`. */
	public get clientWidth(): number { return this._width; }
	public set clientWidth(v: number) { this.width = v; }
	/** Alias for `innerHeight`. */
	public get clientHeight(): number { return this._height; }
	public set clientHeight(v: number) { this.height = v; }
	
	/** Alias for `.on('keydown', callback)`. Setter adds a callback. */
	public get onkeydown(): TCbField<TKeyEvent>{ return this.listeners('keydown') as TEventCb<TKeyEvent>[]; }
	public set onkeydown(cb: TEventCb<TKeyEvent>) { this.on('keydown', cb); }
	
	/** Alias for `.on('keyup', callback)`. Setter adds a callback. */
	public get onkeyup(): TCbField<TKeyEvent>{ return this.listeners('keyup') as TEventCb<TKeyEvent>[]; }
	public set onkeyup(cb: TEventCb<TKeyEvent>) { this.on('keyup', cb); }
	
	/** Alias for `.on('mousedown', callback)`. Setter adds a callback. */
	public get onmousedown(): TCbField<TMouseButtonEvent>{
		return this.listeners('mousedown') as TEventCb<TMouseButtonEvent>[];
	}
	public set onmousedown(cb: TEventCb<TMouseButtonEvent>) { this.on('mousedown', cb); }
	
	/** Alias for `.on('mouseup', callback)`. Setter adds a callback. */
	public get onmouseup(): TCbField<TMouseButtonEvent>{
		return this.listeners('mouseup') as TEventCb<TMouseButtonEvent>[];
	}
	public set onmouseup(cb: TEventCb<TMouseButtonEvent>) { this.on('mouseup', cb); }
	
	/** Alias for `.on('wheel', callback)`. Setter adds a callback. */
	public get onwheel(): TCbField<TMouseScrollEvent>{
		return this.listeners('wheel') as TEventCb<TMouseScrollEvent>[];
	}
	public set onwheel(cb: TEventCb<TMouseScrollEvent>) { this.on('wheel', cb); }
	
	/** Alias for `.on('mousewheel', callback)`. Setter adds a callback. */
	public get onmousewheel(): TCbField<TMouseScrollEvent>{
		return this.listeners('mousewheel') as TEventCb<TMouseScrollEvent>[];
	}
	public set onmousewheel(cb: TEventCb<TMouseScrollEvent>) { this.on('mousewheel', cb); }
	
	/** Alias for `.on('resize', callback)`. Setter adds a callback. */
	public get onresize(): TCbField<TEvent & TSize>{
		return this.listeners('resize') as TEventCb<TEvent & TSize>[];
	}
	public set onresize(cb: TEventCb<TEvent & TSize>) { this.on('resize', cb); }
	
	/** Logical window size. */
	public get size(): TSize {
		const size = glfw.getWindowSize(this._window);
		this._width = size.width;
		this._height = size.height;
		this._pxWidth = size.width * this._ratio;
		this._pxHeight = size.height * this._ratio;
		return size;
	}
	
	public set size({ width, height }: TSize) {
		if (this._width === width && this._height === height) {
			return;
		}
		this._width = width;
		this._height = height;
		this._pxWidth = width * this._ratio;
		this._pxHeight = height * this._ratio;
		glfw.setWindowSize(this._window, width, height);
	}
	
	/** Whether the window is resizable. */
	public get resizable(): boolean {
		const resizable = glfw.getWindowAttrib(this._window, glfw.RESIZABLE);
		this._resizable = !!resizable;
		
		return this._resizable;
	}

	public set resizable(v: boolean) {
		this._resizable = v;
		glfw.setWindowAttrib(this._window, glfw.RESIZABLE, this._resizable ? glfw.TRUE : glfw.FALSE);
	}
	
	/** Window title. */
	public get title(): string { return this._title; }
	public set title(v: string) {
		this._title = v;
		glfw.setWindowTitle(this._window, this._title);
	}
	
	
	/**
	 * Window icon in RGBA format.
	 *
	 * Consider using the @node-3d/image Image implementation. The given image is
	 * vertically flipped unless `noflip` is true.
	 *
	 * @see https://github.com/node-3d/image
	 * @see https://github.com/node-3d/glfw/examples/icon.ts
	 */
	public get icon(): TImageData | null { return this._icon; }
	public set icon(v: TImageData | null | undefined) {
		if (!(v && typeof v === 'object')) {
			this._icon = null;
			return;
		}
		this._icon = v;
		glfw.setWindowIcon(this._window, this._icon);
	}
	
	
	/** Whether the window is going to be closed. */
	public get shouldClose(): boolean { return !!glfw.windowShouldClose(this._window); }
	public set shouldClose(v: boolean) { glfw.setWindowShouldClose(this._window, v ? glfw.TRUE : glfw.FALSE); }
	
	/** Window position X coordinate on the screen. */
	public get x(): number { return this._x; }
	public set x(v: number) {
		if (this._x === v) {
			return;
		}
		this._x = v;
		glfw.setWindowPos(this._window, this._x, this._y);
	}
	
	/** Window position Y coordinate on the screen. */
	public get y(): number { return this._y; }
	public set y(v: number) {
		if (this._y === v) {
			return;
		}
		this._y = v;
		glfw.setWindowPos(this._window, this._x, this._y);
	}
	
	/** Window position coordinates. */
	public get pos(): TPos {
		const pos = glfw.getWindowPos(this._window);
		this._x = pos.x;
		this._y = pos.y;
		return pos;
	}
	public set pos({ x, y }: TPos) {
		if (this._x === x && this._y === y) {
			return;
		}
		this._x = x;
		this._y = y;
		glfw.setWindowPos(this._window, x, y);
	}
	
	public get vsync(): number {
		return this._vsync;
	}
	public set vsync(isVsyncEnabled: number) {
		if (this._vsync === isVsyncEnabled) {
			return;
		}
		
		this._vsync = isVsyncEnabled;
		this.makeCurrent();
		glfw.swapInterval(this._vsync);
	}
	
	
	/** Cursor position coordinates. */
	public get cursorPos(): TPos { return glfw.getCursorPos(this._window); }
	public set cursorPos({ x, y }: TPos) { glfw.setCursorPos(this._window, x, y); }
	
	/** Bound `requestAnimationFrame` method, returns a timer id. */
	public requestAnimationFrame: (callback: (dateNow: number) => void) => NodeJS.Immediate;
	
	/** Bound `cancelAnimationFrame` method. Cancels by id. */
	public cancelAnimationFrame: (id: NodeJS.Immediate) => void;
	
	/**
	 * Bound optimized single-frame method, returns a timer id.
	 *
	 * This method should only have one call per frame. Like `requestAnimationFrame`
	 * it issues a `setImmediate`, but does not need to create a new arrow function
	 * for the frame runner.
	 */
	public frame: (callback: (dateNow: number) => void) => NodeJS.Immediate;
	
	/**
	 * Bound optimized loop method that continuously generates frames with `callback`.
	 *
	 * The returned function breaks the loop.
	 */
	public loop: (callback: (dateNow: number) => void) => () => void;
	
	/** Currently dispatched event, or null outside event dispatch. */
	public event: TEvent | null;
	
	/** Get a monitor having the most overlap with this window. */
	public getCurrentMonitor(): TMonitor | null {
		if (!this._window) {
			return this._primaryDisplay;
		}
		
		let bestOverlap = 0;
		let bestMonitor = null;
		
		const { x: wx, y: wy } = this.pos;
		const { width: ww, height: wh } = this.size;
		
		this._monitors = glfw.getMonitors();
		
		for (const monitor of this._monitors) {
			const { width: mw, height: mh } = monitor;
			const { pos_x: mx, pos_y: my } = monitor;
			const overlap = (
				Math.max(0, Math.min(wx + ww, mx + mw) - Math.max(wx, mx)) *
				Math.max(0, Math.min(wy + wh, my + mh) - Math.max(wy, my))
			);
			if (bestOverlap < overlap) {
				bestOverlap = overlap;
				bestMonitor = monitor;
			}
		}
		
		return bestMonitor;
	}
	
	/**
	 * Get a browser-like rect for this window.
	 *
	 * @see https://developer.mozilla.org/en-US/docs/Web/API/Element/getBoundingClientRect
	 */
	public getBoundingClientRect(): TRect {
		return {
			x: 0,
			y: 0,
			width: this._pxWidth,
			height: this._pxHeight,
			left: 0,
			top: 0,
			right: this._pxWidth,
			bottom: this._pxHeight,
		};
	}
	
	/**
	 * Get key state.
	 *
	 * @see https://www.glfw.org/docs/latest/group__keys.html
	 */
	public getKey(key: number): number { return glfw.getKey(this._window, key); }
	
	/**
	 * Get mouse button state.
	 *
	 * @see https://www.glfw.org/docs/latest/group__buttons.html
	 */
	public getMouseButton(button: number): number { return glfw.getMouseButton(this._window, button); }
	
	/**
	 * Get window attribute.
	 *
	 * @see https://www.glfw.org/docs/latest/window_guide.html#window_attribs
	 */
	public getWindowAttrib(attrib: number): number { return glfw.getWindowAttrib(this._window, attrib); }
	
	/** Set input mode option. */
	public setInputMode(mode: number, value: number): void { glfw.setInputMode(this._window, mode, value); }
	
	/** Swap the front and back buffers of the window. */
	public swapBuffers(): void { glfw.swapBuffers(this._window); }
	
	/** Make this window's GL context current. */
	public makeCurrent(): void { glfw.makeContextCurrent(this._window); }
	
	/** Destroy the GLFW window. */
	public destroy(): void { glfw.destroyWindow(this._window); }
	
	/** Minimize the window. */
	public iconify(): void { glfw.iconifyWindow(this._window); }
	
	/** Restore the window if it was previously iconified or maximized. */
	public restore(): void { glfw.restoreWindow(this._window); }
	
	/** Hide the window. */
	public hide(): void { glfw.hideWindow(this._window); }
	
	/** Show the window if it is hidden. */
	public show(): void { glfw.showWindow(this._window); }
	
	
	public emit(type: string, event: TEvent): boolean {
		if (type === 'keydown' || type === 'keyup') {
			const keyEvent = event as TKeyEvent;
			const glfwCode = keyEvent.which;
			
			keyEvent.which = extraCodes[glfwCode] || glfwCode;
			event.keyCode = keyEvent.which;
			keyEvent.key = (
				keyEvent.charCode && String.fromCodePoint(keyEvent.charCode) ||
				keyEvent.code ||
				keyNames[glfwCode] ||
				'?'
			);
			keyEvent.code = (
				codeNames[glfwCode] ||
				(keyEvent.code && `Key${keyEvent.code.toUpperCase()}`) ||
				'UNKNOWN'
			);
		}
		
		event.target = this;
		event.type = type;
		event.preventDefault = emptyFunction;
		event.stopPropagation = emptyFunction;
		
		this.makeCurrent();
		
		this.event = event;
		const isHandled = super.emit(type, event);
		this.event = null;
		
		return isHandled;
	}
	
	
	/** Draw one frame by polling events, calling `callback`, and swapping buffers. */
	public drawWindow(cb: (dateNow: number) => void): void { glfw.drawWindow(this._window, cb); }
	
	/** Alias for `emit`; event type is expected inside the event object. */
	public dispatchEvent(event: TEvent): void { this.emit(event.type, event); }
	
	/** Alias for `on`. */
	public addEventListener(name: string, callback: TEventCb<TEvent>): this {
		this.on(name, callback);
		return this;
	}
	
	/** Alias for `removeListener`. */
	public removeEventListener(name: string, callback: TEventCb<TEvent>): void { this.removeListener(name, callback); }
	
	public static exit(): never {
		// oxlint-disable-next-line unicorn/no-process-exit
		process.exit(0);
	}
	
	// Previous windowed-mode X coordinate restored after leaving fullscreen-like modes.
	private _prevX: number | null = null;
	
	// Previous windowed-mode Y coordinate restored after leaving fullscreen-like modes.
	private _prevY: number | null = null;
	
	// Previous windowed-mode width restored after leaving fullscreen-like modes.
	private _prevWidth: number | null = null;
	
	// Previous windowed-mode height restored after leaving fullscreen-like modes.
	private _prevHeight: number | null = null;
	
	// Previous decoration state restored after leaving borderless mode.
	private _prevDecorated: boolean | null = null;
	
	// Requested or detected OpenGL major version.
	private _major: number = 0;
	
	// Requested or detected OpenGL minor version.
	private _minor: number = 0;
	
	// Detected OpenGL context revision.
	private _rev: number = 0;
	
	// Detected OpenGL profile value.
	private _prof: number = 0;
	
	// Cached window title mirrored to GLFW.
	private _title: string = '';
	
	// Cached window icon mirrored to GLFW.
	private _icon: TImageData | null = null;
	
	// Hidden GLFW window handles kept for mode switches.
	private _modeCache: Partial<Record<TWindowMode, TWindowHandle | null>> = {};
	
	// Logical window width.
	private _width: number = DEFAULT_WIDTH;
	
	// Physical framebuffer width.
	private _pxWidth: number = DEFAULT_WIDTH;
	
	// Logical window height.
	private _height: number = DEFAULT_HEIGHT;
	
	// Physical framebuffer height.
	private _pxHeight: number = DEFAULT_HEIGHT;
	
	// Ratio between physical framebuffer pixels and logical window pixels.
	private _ratio: number = 1;
	
	// User hook called immediately before a GLFW window is created.
	private _onBeforeWindow: TWindowOpts['onBeforeWindow'];
	
	// Current monitor index.
	private _display: number = 0;
	
	// Last known monitor list.
	private _monitors: readonly TMonitor[] = [];
	
	// Primary monitor fallback used before a native window exists.
	private _primaryDisplay: TMonitor | null = null;
	
	// Current swap interval value.
	private _vsync: number = 0;
	
	// Whether fullscreen windows iconify on focus loss.
	private _autoIconify: boolean = true;
	
	// Current logical window mode.
	private _mode: TWindowMode | undefined;
	
	// Whether GLFW creates decorated windows for this instance.
	private _decorated: boolean = true;
	
	// Multisample antialiasing level passed to GLFW.
	private _msaa: number = 0;
	
	// Current resizable flag mirrored to GLFW.
	private _resizable: boolean = true;
	
	// Event bridge passed into the native GLFW addon.
	private _emitter: TNativeEmitter;
	
	// Current native GLFW window handle.
	private _window!: TWindowHandle; // assigned in constructor during `this.mode = x`
	
	// Current window X coordinate.
	private _x: number = 0;
	
	// Current window Y coordinate.
	private _y: number = 0;
	
	// Cached frame callback used by the optimized frame path.
	private _rafCb: (dateNow: number) => void;
	
	// Cached draw runner used by the optimized frame path.
	private _drawWithCb: () => void;
	
	// Initialize cached fields from constructor options before creating a GLFW window.
	private _readOptions(opts: TWindowOpts): void {
		this._major = opts.major === undefined ? 2 : opts.major;
		this._minor = opts.minor === undefined ? 1 : opts.minor;
		
		this._width = opts.width || 1280;
		this._pxWidth = this._width;
		this._height = opts.height || 720;
		this._pxHeight = this._height;
		
		this._onBeforeWindow = opts.onBeforeWindow;
		this._display = opts.display ?? 0;
		this._monitors = glfw.getMonitors();
		this._primaryDisplay = this._monitors.find((d) => d.is_primary) || null;
		
		this._vsync = opts.vsync ? 1 : 0; // 0 for vsync off
		this._autoIconify = opts.autoIconify !== false;
		
		if (opts.decorated !== undefined) {
			this._decorated = opts.decorated;
		}
		
		this._msaa = opts.msaa || 0;

		this._resizable = opts.resizable !== false;
	}
	
	
	// Apply GLFW window hints that must be set before native window creation.
	private _applyWindowHints(): void {
		glfw.windowHint(glfw.CONTEXT_VERSION_MAJOR, this._major);
		glfw.windowHint(glfw.CONTEXT_VERSION_MINOR, this._minor);
		
		glfw.windowHint(glfw.RESIZABLE, this._resizable ? glfw.TRUE : glfw.FALSE);
		glfw.windowHint(glfw.VISIBLE, glfw.TRUE);
		glfw.windowHint(glfw.RED_BITS, 8);
		glfw.windowHint(glfw.GREEN_BITS, 8);
		glfw.windowHint(glfw.BLUE_BITS, 8);
		glfw.windowHint(glfw.DEPTH_BITS, 24);
		glfw.windowHint(glfw.REFRESH_RATE, 0);
		glfw.windowHint(glfw.DOUBLEBUFFER, glfw.TRUE);
		glfw.windowHint(glfw.AUTO_ICONIFY, this._autoIconify ? glfw.TRUE : glfw.FALSE);
		glfw.windowHint(glfw.DECORATED, this._decorated ? glfw.TRUE : glfw.FALSE);
		glfw.windowHint(glfw.SAMPLES, this._msaa);
	}
	
	
	// Compute device pixel ratio after the native window exists.
	private _syncFramebufferRatio(): void {
		const sizeWin = this.size;
		const sizeFB  = this.framebufferSize;
		
		this._ratio = sizeFB.width / sizeWin.width;
	}
	
	
	// Apply initial title and icon values after the native window exists.
	private _applyInitialAppearance(opts: TWindowOpts): void {
		this.icon = opts.icon;
		
		if (opts.title) {
			this.title = opts.title;
		} else {
			const dirname = process.cwd();
			this.title = dirname;
		}
	}
	
	
	// Read OpenGL context attributes that are only available after window creation.
	private _readContextAttributes(): void {
		this._major = glfw.getWindowAttrib(this._window, glfw.CONTEXT_VERSION_MAJOR);
		this._minor = glfw.getWindowAttrib(this._window, glfw.CONTEXT_VERSION_MINOR);
		this._rev = glfw.getWindowAttrib(this._window, glfw.CONTEXT_REVISION);
		this._prof = glfw.getWindowAttrib(this._window, glfw.OPENGL_PROFILE);
	}
	
	
	// Keep cached position and size fields aligned with native events.
	private _bindStateEvents(): void {
		this.on('window_pos', ({ x, y }) => {
			this._x = x;
			this._y = y;
		});
		
		this.on('wresize', ({ width, height }) => {
			this._width = width;
			this._height = height;
		});
		
		this.on('resize', ({ width, height }) => {
			this._pxWidth = width;
			this._pxHeight = height;
		});
	}
	
	// Create a new native window according to the current display mode.
	private _create(): void {
		if (this._mode === 'windowed') {
			glfw.windowHint(glfw.DECORATED, this._decorated ? glfw.TRUE : glfw.FALSE);
			
			if (this._onBeforeWindow) {
				this._onBeforeWindow(this, glfw);
			}
			
			this._window = glfw.createWindow(
				this._width,
				this._height,
				this._emitter,
				this._title ?? undefined
			);
		} else if (this._mode === 'borderless') {
			this._prevDecorated = this._decorated;
			this._decorated = false;
			
			glfw.windowHint(glfw.DECORATED, glfw.FALSE);
			
			if (this._onBeforeWindow) {
				this._onBeforeWindow(this, glfw);
			}
			
			this._window = glfw.createWindow(
				this._width,
				this._height,
				this._emitter,
				this._title ?? undefined
			);
		} else if (this._mode === 'fullscreen') {
			this._adjustFullscreen();
			
			if (this._onBeforeWindow) {
				this._onBeforeWindow(this, glfw);
			}
			
			this._window = glfw.createWindow(
				this._width,
				this._height,
				this._emitter,
				this._title ?? undefined,
				this._display
			);
		} else {
			throw new Error(`Not supported display mode: '${this._mode}'.`);
		}
		
		if (!this._window) {
			throw new Error('Failed to open a new GLFW Window');
		}
		
	}
	
	// Measure how far a monitor mode area is from the current window area.
	private _areaDiff(mode: TMonitorMode): number {
		return Math.abs(mode.width * mode.height - this._width * this._height);
	}
	
	
	// Check whether a monitor mode exactly matches the current logical window size.
	private _sizeEqual(mode: TMonitorMode): boolean {
		return mode.width === this._width && mode.height === this._height;
	}
	
	
	// Keep monitor modes with the nearest area to the current logical window size.
	private _sortByAreaDiff(modes: readonly TMonitorMode[]): readonly TMonitorMode[] {
		const sorted = modes.toSorted((a, b) => this._areaDiff(a) - this._areaDiff(b));
		const best = this._areaDiff(sorted[0]);
		return sorted.filter((mode) => this._areaDiff(mode) === best);
	}
	
	
	// Sort monitor modes by nearest area to the current logical window size.
	private _sortByRate(modes: readonly TMonitorMode[]): readonly TMonitorMode[] {
		return modes.toSorted((a, b) => this._areaDiff(a) - this._areaDiff(b));
	}
	
	
	// Choose the fullscreen monitor mode and apply its refresh rate hint.
	private _adjustFullscreen(): void {
		const mode = (() => {
			const modes = this._monitors[this._display].modes;
			const exact = modes.filter((mode) => this._sizeEqual(mode));
			const chosen = (exact.length > 0 ? exact : this._sortByAreaDiff(modes));
			return chosen.toSorted((a, b) => b.rate - a.rate)[0];
		})();
		
		this._width = mode.width;
		this._height = mode.height;
		
		glfw.windowHint(glfw.REFRESH_RATE, mode.rate);
	}
}
