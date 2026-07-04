import assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import { getPlatform } from '@node-3d/addon-tools';
import { Document } from './document.ts';

const documentProperties = [
	'body',
	'ratio',
	'devicePixelRatio',
	'innerWidth',
	'innerHeight',
	'clientWidth',
	'clientHeight',
	'onkeydown',
	'onkeyup',
	'style',
	'context',
] as const;

const documentMethods = [
	'getContext',
	'getElementById',
	'getElementsByTagName',
	'createElementNS',
	'createElement',
	'dispatchEvent',
	'addEventListener',
	'removeEventListener',
	'requestAnimationFrame',
] as const;

const testProperty = (getInstance: () => Document, prop: string): void => {
	it(`#${prop} property exposed`, () => {
		assert.notStrictEqual(Reflect.get(getInstance(), prop), undefined);
	});
};

const testMethod = (getInstance: () => Document, method: string): void => {
	it(`#${method}() method exposed`, () => {
		assert.strictEqual(typeof Reflect.get(getInstance(), method), 'function');
	});
};

if (getPlatform() === 'linux') {
	describe('Document', () => {
		let instance: Document | null = null;
		const getInstance = (): Document => {
			assert.ok(instance);
			return instance;
		};

		before(() => {
			instance = new Document();
		});

		after(() => {
			instance?.destroy();
		});

		it('can be created', () => {
			assert.ok(instance instanceof Document);
		});

		for (const prop of documentProperties) {
			testProperty(getInstance, prop);
		}

		for (const method of documentMethods) {
			testMethod(getInstance, method);
		}
	});
}
