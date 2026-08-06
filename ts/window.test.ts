import assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import { getPlatform } from '@node-3d/addon-tools';
import { GlfwWindow } from './window.ts';

const windowProperties = [
	'handle',
	'width',
	'height',
	'w',
	'h',
	'wh',
	'size',
	'title',
	'icon',
	'msaa',
	'version',
	'shouldClose',
	'platformWindow',
	'platformContext',
	'pos',
	'framebufferSize',
	'currentContext',
	'cursorPos',
	'vsync',
	'swapInterval',
] as const;

const windowMethods = [
	'getKey',
	'getMouseButton',
	'getWindowAttrib',
	'setInputMode',
	'swapBuffers',
	'makeCurrent',
	'destroy',
	'iconify',
	'restore',
	'hide',
	'show',
] as const;

const testProperty = (getInstance: () => GlfwWindow, prop: string): void => {
	it(`#${prop} property exposed`, () => {
		assert.notStrictEqual(Reflect.get(getInstance(), prop), undefined);
	});
};

const testMethod = (getInstance: () => GlfwWindow, method: string): void => {
	it(`#${method}() method exposed`, () => {
		assert.strictEqual(typeof Reflect.get(getInstance(), method), 'function');
	});
};

if (getPlatform() === 'linux') {
	describe('GlfwWindow', () => {
		let instance: GlfwWindow | null = null;
		const getInstance = (): GlfwWindow => {
			assert.ok(instance);
			return instance;
		};

		before(() => {
			instance = new GlfwWindow();
		});

		after(() => {
			instance?.destroy();
		});

		it('can be created', () => {
			assert.ok(instance instanceof GlfwWindow);
		});

		for (const prop of windowProperties) {
			testProperty(getInstance, prop);
		}

		for (const method of windowMethods) {
			testMethod(getInstance, method);
		}
	});
}
