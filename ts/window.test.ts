import assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import { getPlatform } from '@node-3d/addon-tools';
import { Window } from './window.ts';

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

const testProperty = (getInstance: () => Window, prop: string): void => {
	it(`#${prop} property exposed`, () => {
		assert.notStrictEqual(Reflect.get(getInstance(), prop), undefined);
	});
};

const testMethod = (getInstance: () => Window, method: string): void => {
	it(`#${method}() method exposed`, () => {
		assert.strictEqual(typeof Reflect.get(getInstance(), method), 'function');
	});
};

if (getPlatform() === 'linux') {
	describe('Window', () => {
		let instance: Window | null = null;
		const getInstance = (): Window => {
			assert.ok(instance);
			return instance;
		};
		
		before(() => {
			instance = new Window();
		});
		
		after(() => {
			instance?.destroy();
		});
		
		it('can be created', () => {
			assert.ok(instance instanceof Window);
		});
		
		for (const prop of windowProperties) {
			testProperty(getInstance, prop);
		}
		
		for (const method of windowMethods) {
			testMethod(getInstance, method);
		}
	});
}
