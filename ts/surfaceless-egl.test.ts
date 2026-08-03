import assert from 'node:assert/strict';
import { it } from 'node:test';
import { native } from './native.ts';

it(
	'renders and reads pixels through Mesa surfaceless EGL',
	{ skip: process.platform !== 'darwin' },
	() => {
		const result = native.testSurfacelessEgl(16, 16);
		const firstPixel = [...result.pixels.subarray(0, 4)];

		assert.strictEqual(result.mode, 'surfaceless-egl');
		assert.strictEqual(result.width, 16);
		assert.strictEqual(result.height, 16);
		assert.strictEqual(result.surfaceless, true);
		assert.deepStrictEqual(firstPixel, [255, 0, 0, 255]);
		assert.strictEqual(result.vendor, 'Mesa');
		assert.match(result.renderer, /llvmpipe/u);
		assert.match(result.version, /^OpenGL ES/u);
		assert.match(result.clientExtensions, /\bEGL_MESA_platform_surfaceless\b/u);
	},
);
