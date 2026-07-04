import { getLogger } from '@node-3d/addon-tools';
import { emptyFunction, ESC_KEY, F_KEY } from './constants.ts';
import { FakeImage } from './fake-image.ts';
import { glfw } from './core.ts';
import { Window } from './window.ts';
import type { TCbVoid, TSize, TWebgl } from './types.ts';

const logger = getLogger('glfw');

export type TDocumentOpts = ConstructorParameters<typeof Window>[0] &
	Readonly<
		Partial<{
			/**
			 * Whether the window should ignore default quit signals.
			 *
			 * Examples: process `SIGINT`, document `quit`, and ESC press if `autoEsc` is enabled.
			 */
			ignoreQuit: boolean;
			/**
			 * Whether the window has default fullscreen key handlers.
			 *
			 * * CTRL+F - borderless fullscreen window.
			 * * CTRL+ALT+F - real, exclusive fullscreen mode.
			 * * CTRL+SHIFT+F - back to windowed.
			 */
			autoFullscreen: boolean;
			/**
			 * Handle ESC key to close the window automatically.
			 *
			 * Does nothing if `ignoreQuit` is enabled.
			 */
			autoEsc: boolean;
		}>
	>;

type TImageConstructor = new () => {
	src?: string;
	complete?: boolean;
	on?: (...args: unknown[]) => void;
	onload?: (...args: unknown[]) => void;
	onerror?: (...args: unknown[]) => void;
};

type TMutableWebgl = TWebgl & {
	canvas?: Document;
	init?: TCbVoid;
	data?: unknown;
};

type TCanvasStub = Readonly<{
	width: number;
	height: number;
	getContext: (kind: string) => TMutableWebgl | InstanceType<typeof Document.Image> | null;
	readonly data: unknown;
	onkeydown: TCbVoid;
	onkeyup: TCbVoid;
	onmousedown: TCbVoid;
	onmouseup: TCbVoid;
	onwheel: TCbVoid;
	onmousewheel: TCbVoid;
	onresize: TCbVoid;
	dispatchEvent: TCbVoid;
	addEventListener: TCbVoid;
	removeEventListener: TCbVoid;
}>;

/**
 * Web-like Document.
 *
 * Document extends Window to provide an additional web-style compatibility
 * layer. It mimics the behavior and properties of a browser `window.document`,
 * while still being a Window. It is intentionally incomplete: callers still
 * provide the Image class and WebGL implementation they want to use.
 */
export class Document extends Window {
	public static Image: TImageConstructor;
	public static webgl: TMutableWebgl | null;
	public static isWebglInited: boolean;

	private _isCanvasRequested: boolean;

	/**
	 * Set Image implementation.
	 *
	 * Also sets `global.HTMLImageElement`.
	 */
	public static setImage(Image: TImageConstructor): void {
		this.Image = Image;
		global.HTMLImageElement = Image as typeof global.HTMLImageElement;
	}

	/**
	 * Set WebGL implementation.
	 */
	public static setWebgl(webgl: TMutableWebgl | null): void {
		this.webgl = webgl;
		this.isWebglInited = false;
	}

	public constructor(opts: TDocumentOpts = {}) {
		super(opts);
		this._isCanvasRequested = false;

		if (Document.webgl && !Document.isWebglInited) {
			try {
				if (typeof Document.webgl.init === 'function') {
					Document.webgl.init();
				}
			} catch {
				logger.warn('WebGL `init()` call failed, but it may still work.');
			}
			Document.isWebglInited = true;
		}
		if (Document.webgl) {
			Document.webgl.canvas = this;
		}

		this.on('mousedown', (e) => {
			this.emit('pointerdown', e);
		});
		this.on('mouseup', (e) => {
			this.emit('pointerup', e);
		});
		this.on('mousemove', (e) => {
			this.emit('pointermove', e);
		});

		if (!opts.ignoreQuit) {
			const isUnix = process.platform !== 'win32';
			if (isUnix && !process.listeners('SIGINT').includes(Document.exit)) {
				process.on('SIGINT', Document.exit);
			}

			this.on('quit', () => Window.exit());

			if (opts.autoEsc) {
				this.on('keydown', (e) => e.keyCode === ESC_KEY && Window.exit());
			}
		}

		if (opts.autoFullscreen) {
			this.on('keydown', (e) => {
				if (e.keyCode === F_KEY && e.ctrlKey && e.shiftKey) {
					this.mode = 'windowed';
				} else if (e.keyCode === F_KEY && e.ctrlKey && e.altKey) {
					this.mode = 'fullscreen';
				} else if (e.keyCode === F_KEY && e.ctrlKey) {
					this.mode = 'borderless';
				}
			});
		}
	}

	/** Set `glfw.CURSOR` mode to `glfw.CURSOR_DISABLED`. */
	public setPointerCapture = (): void => {
		this.setInputMode(glfw.CURSOR, glfw.CURSOR_DISABLED);
	};

	/** Set `glfw.CURSOR` mode to `glfw.CURSOR_NORMAL`. */
	public releasePointerCapture = (): void => {
		this.setInputMode(glfw.CURSOR, glfw.CURSOR_NORMAL);
	};

	public makeCurrent(): void {
		if (Document.webgl) {
			Document.webgl.canvas = this;
		}
		super.makeCurrent();
	}

	/** Returns `this`. */
	public get body(): Document {
		return this;
	}

	/**
	 * Mimics the web element `style` property.
	 *
	 * Only `width` and `height` matter.
	 */
	public get style(): TSize {
		const getWidth = (): number => this.innerWidth;
		const setWidth = (value: string): void => {
			this.width = Number.parseInt(value, 10) * this.devicePixelRatio;
		};
		const getHeight = (): number => this.innerHeight;
		const setHeight = (value: string): void => {
			this.height = Number.parseInt(value, 10) * this.devicePixelRatio;
		};

		return {
			get width(): number {
				return getWidth();
			},
			set width(value: string) {
				setWidth(value);
			},
			get height(): number {
				return getHeight();
			},
			set height(value: string) {
				setHeight(value);
			},
		};
	}

	/** Returns `Document.webgl`, set through `Document.setWebgl`. */
	public get context(): TWebgl | null {
		return Document.webgl;
	}

	/** Returns `Document.webgl`, set through `Document.setWebgl`. */
	public getContext(kind: string): TMutableWebgl | InstanceType<typeof Document.Image> | null {
		return kind === '2d' ? new Document.Image() : Document.webgl;
	}

	/** Returns `this`. */
	public getRootNode(): Document {
		return this;
	}

	/** Returns `this`. */
	public getElementById(): Document {
		return this;
	}

	/** Returns `this`. */
	public querySelector(): Document {
		return this;
	}

	/** Returns an array containing `this`. */
	public querySelectorAll(): readonly Document[] {
		return [this];
	}

	/** Returns an array containing `this`. */
	public getElementsByTagName(): readonly Document[] {
		return [this];
	}

	/** Does nothing. */
	public appendChild(): void {
		/* nop */
	}

	/** Does nothing. */
	public append(): void {
		/* nop */
	}

	/** Returns the result of `createElement(name)`. */
	public createElementNS(
		_0: unknown,
		name: string,
	): Document | InstanceType<typeof Document.Image> | TCanvasStub | null {
		return this.createElement(name);
	}

	/**
	 * Fake `createElement`.
	 *
	 * For `canvas`, returns `this` on the first call, then returns canvas-like
	 * objects capable of using 2D or 3D context. This supports web APIs like
	 * three.js, which create additional canvases.
	 *
	 * For `img`, returns `new Document.Image()`.
	 */
	public createElement(
		nameRaw: string,
	): Document | InstanceType<typeof Document.Image> | TCanvasStub | null {
		const name = nameRaw.toLowerCase();

		if (name.includes('img')) {
			return new Document.Image();
		}

		if (name.includes('canvas')) {
			if (!this._isCanvasRequested) {
				this._isCanvasRequested = true;
				return this;
			}

			const getContext = (
				kind: string,
			): TMutableWebgl | InstanceType<typeof Document.Image> | null => this.getContext(kind);
			let ctx: TMutableWebgl | InstanceType<typeof Document.Image> | null = null;

			return {
				width: this.width,
				height: this.height,

				getContext(kind) {
					ctx = getContext(kind);
					return ctx;
				},

				get data() {
					return ctx && 'data' in ctx ? ctx.data : undefined;
				},

				onkeydown: emptyFunction,
				onkeyup: emptyFunction,
				onmousedown: emptyFunction,
				onmouseup: emptyFunction,
				onwheel: emptyFunction,
				onmousewheel: emptyFunction,
				onresize: emptyFunction,

				dispatchEvent: emptyFunction,
				addEventListener: emptyFunction,
				removeEventListener: emptyFunction,
			};
		}

		return null;
	}
}

global.HTMLCanvasElement = Document as unknown as typeof global.HTMLCanvasElement;

Document.setImage(FakeImage);
Document.setWebgl(null);
