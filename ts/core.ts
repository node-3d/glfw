// oxlint-disable unicorn/prefer-export-from
import { createLogger } from '@node-3d/addon-tools';
import { native } from './native.ts';

// oxlint-disable no-underscore-dangle

// Add deps dll dirs

createLogger({ name: 'glfw' });

const nodeGlobal = globalThis as unknown as { __isGlfwInited?: boolean };

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
 * Use `Window` and `Document` for higher-level wrappers.
 */
export const glfw = native;
