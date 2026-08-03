// oxlint-disable no-console node/no-sync node/no-process-env
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { describe, it } from 'node:test';

type TProbe = Readonly<{
	name: string;
	mode: 'raw-auto' | 'raw-manual' | 'window-auto' | 'window-manual';
	platform?: 'cocoa' | 'null';
	context?: 'egl' | 'osmesa';
	client?: 'opengl' | 'gles';
	cleanup?: boolean;
}>;

type TProbeReport = Readonly<{
	ok: boolean;
	error?: string;
	details?: unknown;
}>;

const reportPrefix = '__NODE_3D_GLFW_WINDOW_PROBE__';
const childSource = String.raw`
const reportPrefix = '${reportPrefix}';

const asMessage = (error) => error instanceof Error ? error.name + ': ' + error.message : String(error);

const writeReport = (report) => {
	console.log(reportPrefix + JSON.stringify(report));
};

const setupInitHints = (glfw, probe) => {
	if (probe.platform === 'cocoa') {
		glfw.initHint(glfw.PLATFORM, glfw.PLATFORM_COCOA);
	} else if (probe.platform === 'null') {
		glfw.initHint(glfw.PLATFORM, glfw.PLATFORM_NULL);
	}
};

const setupWindowHints = (glfw, probe) => {
	glfw.windowHint(glfw.VISIBLE, glfw.FALSE);
	glfw.windowHint(glfw.STENCIL_BITS, 0);
	glfw.windowHint(glfw.DEPTH_BITS, 0);
	glfw.windowHint(glfw.SAMPLES, 0);

	if (probe.context === 'egl') {
		glfw.windowHint(glfw.CONTEXT_CREATION_API, glfw.EGL_CONTEXT_API);
	} else if (probe.context === 'osmesa') {
		glfw.windowHint(glfw.CONTEXT_CREATION_API, glfw.OSMESA_CONTEXT_API);
	}

	if (probe.client === 'gles') {
		glfw.windowHint(glfw.CLIENT_API, glfw.OPENGL_ES_API);
		glfw.windowHint(glfw.CONTEXT_VERSION_MAJOR, 3);
		glfw.windowHint(glfw.CONTEXT_VERSION_MINOR, 0);
	}
};

const initManual = async (probe) => {
	globalThis.__isGlfwInited = true;
	const { native: glfw } = await import(process.env.NODE_3D_GLFW_NATIVE_URL);
	setupInitHints(glfw, probe);

	if (!glfw.init()) {
		throw new Error('Failed to initialize GLFW');
	}

	glfw.defaultWindowHints();
	return glfw;
};

const createRawWindow = (glfw, probe) => {
	setupWindowHints(glfw, probe);
	const emitter = { emit() {} };
	const window = glfw.createWindow(64, 64, emitter, probe.name);
	if (!window) {
		throw new Error('GLFW returned an empty window handle');
	}
	return window;
};

const readCurrentFrame = (glfw) => {
	const frame = glfw.testCurrentContextFrame(16, 16);
	const firstPixel = [...frame.pixels.subarray(0, 4)];
	if (firstPixel.join(',') !== '255,0,0,255') {
		throw new Error('Unexpected first pixel: ' + firstPixel.join(','));
	}
	return {
		...frame,
		pixels: undefined,
		firstPixel,
	};
};

const probe = JSON.parse(process.env.NODE_3D_GLFW_PROBE);
let glfw = null;
let window = null;
let wrappedWindow = null;

try {
	if (probe.mode === 'raw-auto') {
		const mod = await import(process.env.NODE_3D_GLFW_INDEX_URL);
		glfw = mod.glfw;
		window = createRawWindow(glfw, probe);
		writeReport({
			ok: true,
			details: {
				platform: glfw.getPlatform(),
				framebufferSize: glfw.getFramebufferSize(window),
				currentContext: !!glfw.getCurrentContext(window),
				frame: readCurrentFrame(glfw),
			},
		});
	} else if (probe.mode === 'raw-manual') {
		glfw = await initManual(probe);
		window = createRawWindow(glfw, probe);
		writeReport({
			ok: true,
			details: {
				platform: glfw.getPlatform(),
				framebufferSize: glfw.getFramebufferSize(window),
				currentContext: !!glfw.getCurrentContext(window),
				frame: readCurrentFrame(glfw),
			},
		});
	} else if (probe.mode === 'window-auto') {
		const mod = await import(process.env.NODE_3D_GLFW_INDEX_URL);
		glfw = mod.glfw;
		wrappedWindow = new mod.GlfwWindow({
			width: 64,
			height: 64,
			title: probe.name,
			onBeforeWindow(_window, currentGlfw) {
				setupWindowHints(currentGlfw, probe);
			},
		});
		writeReport({
			ok: true,
			details: {
				version: wrappedWindow.version,
				framebufferSize: wrappedWindow.framebufferSize,
				currentContext: !!wrappedWindow.currentContext,
				frame: readCurrentFrame(glfw),
			},
		});
	} else if (probe.mode === 'window-manual') {
		glfw = await initManual(probe);
		const mod = await import(process.env.NODE_3D_GLFW_INDEX_URL);
		wrappedWindow = new mod.GlfwWindow({
			width: 64,
			height: 64,
			title: probe.name,
			onBeforeWindow(_window, currentGlfw) {
				setupWindowHints(currentGlfw, probe);
			},
		});
		writeReport({
			ok: true,
			details: {
				version: wrappedWindow.version,
				framebufferSize: wrappedWindow.framebufferSize,
				currentContext: !!wrappedWindow.currentContext,
				frame: readCurrentFrame(glfw),
			},
		});
	} else {
		throw new Error('Unknown probe mode: ' + probe.mode);
	}
} catch (error) {
	writeReport({ ok: false, error: asMessage(error) });
	process.exitCode = 1;
} finally {
	if (probe.cleanup !== false) {
		try {
			if (wrappedWindow) {
				wrappedWindow.destroy();
			} else if (glfw && window) {
				glfw.destroyWindow(window);
			}
		} catch {
			// Ignore cleanup errors after failed native probes.
		}
		try {
			glfw?.terminate();
		} catch {
			// Ignore cleanup errors after failed native probes.
		}
	}
}
`;
const platformProbes = (): readonly TProbe[] => {
	if (process.platform === 'darwin') {
		return [
			{
				name: 'raw/manual/null/egl/gles',
				mode: 'raw-manual',
				platform: 'null',
				context: 'egl',
				client: 'gles',
			},
			{
				name: 'window/manual/null/egl/gles/atexit',
				mode: 'window-manual',
				platform: 'null',
				context: 'egl',
				client: 'gles',
				cleanup: false,
			},
		];
	}

	const probes: TProbe[] = [
		{ name: 'raw/auto/default', mode: 'raw-auto' },
		{ name: 'window/auto/default', mode: 'window-auto' },
	];

	return probes;
};

const readReport = (stdout: string): TProbeReport | null => {
	const line = stdout.split(/\r?\n/u).find((currentLine) => currentLine.startsWith(reportPrefix));

	if (!line) {
		return null;
	}

	return JSON.parse(line.slice(reportPrefix.length)) as TProbeReport;
};

const uniq = (values: readonly string[]): readonly string[] => [...new Set(values)];

const extractGlfwErrors = (output: string): readonly string[] =>
	uniq(
		output
			.split(/\r?\n/u)
			.map((line) => line.trim())
			.filter((line) => line.startsWith('GLFW Error ')),
	);

const exitText = (child: ReturnType<typeof spawnSync>): string => {
	if (child.error) {
		return child.error.message;
	}

	if (child.signal) {
		return `signal ${child.signal}`;
	}

	return `exit ${child.status}`;
};

const runProbe = (probe: TProbe): string | null => {
	const child = spawnSync(process.execPath, ['--input-type=module', '--eval', childSource], {
		encoding: 'utf8',
		env: {
			...process.env,
			NODE_3D_GLFW_PROBE: JSON.stringify(probe),
			NODE_3D_GLFW_INDEX_URL: new URL('index.ts', import.meta.url).href,
			NODE_3D_GLFW_NATIVE_URL: new URL('native.ts', import.meta.url).href,
		},
	});
	const stdout = child.stdout || '';
	const stderr = child.stderr || '';
	const report = readReport(stdout);

	if (child.status === 0 && report?.ok) {
		const glfwErrors = extractGlfwErrors(`${stdout}\n${stderr}`);
		if (glfwErrors.length > 0) {
			return `${probe.name}: ${glfwErrors.join(' | ')}`;
		}

		console.log(`[glfw] ok ${probe.name}: ${JSON.stringify(report.details)}`);
		return null;
	}

	const glfwErrors = extractGlfwErrors(`${stdout}\n${stderr}`);
	const reason = report?.error || exitText(child);
	const glfwText = glfwErrors.length > 0 ? `; ${glfwErrors.join(' | ')}` : '';
	return `${probe.name}: ${reason}${glfwText}`;
};

describe('GLFW runtime window creation', () => {
	for (const probe of platformProbes()) {
		it(probe.name, () => {
			const error = runProbe(probe);
			assert.strictEqual(error, null);
		});
	}
});
