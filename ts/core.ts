// oxlint-disable unicorn/prefer-export-from
import { createLogger } from '@node-3d/addon-tools';
import { native } from './native.ts';

// oxlint-disable no-underscore-dangle

// Add deps dll dirs

createLogger({ name: 'glfw' });

const nodeGlobal = globalThis as unknown as { __isGlfwInited?: boolean };

export type TSurfacelessEglFrame = Readonly<{
	mode: 'surfaceless-egl';
	width: number;
	height: number;
	major: number;
	minor: number;
	surfaceless: boolean;
	pixels: Buffer;
	vendor: string;
	renderer: string;
	version: string;
	clientExtensions: string;
}>;

// Initialize GLFW
if (!nodeGlobal.__isGlfwInited) {
	if (!native.init()) {
		throw new Error('Failed to initialize GLFW');
	}

	// OpenGL window default hints
	native.defaultWindowHints();
	nodeGlobal.__isGlfwInited = true;
}

/**
 * Raw GLFW native bindings.
 *
 * Use `GlfwWindow` for a native window wrapper. Browser-style wrappers live in @node-3d/core.
 */
export const glfw = native;

/**
 * Render one RGBA frame through Mesa surfaceless EGL and return the pixels.
 *
 * This is the non-window OpenGL path used by macOS CI where Cocoa/NSGL windows
 * are unavailable on GitHub-hosted runners.
 */
export const renderSurfacelessEglFrame = (width = 64, height = 64): TSurfacelessEglFrame =>
	native.renderSurfacelessEglFrame(width, height);
