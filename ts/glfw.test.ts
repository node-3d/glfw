import assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as glfwModule from './index.ts';

describe('GLFW exports', () => {
	it('exports the native GLFW surface', () => {
		assert.strictEqual(typeof glfwModule.glfw, 'object');
	});
	
	it('exports Window', () => {
		assert.strictEqual(typeof glfwModule.Window, 'function');
	});
	
	it('exports Document', () => {
		assert.strictEqual(typeof glfwModule.Document, 'function');
	});
	
	it('exports key maps', () => {
		assert.strictEqual(typeof glfwModule.keyNames, 'object');
		assert.strictEqual(typeof glfwModule.codeNames, 'object');
		assert.strictEqual(typeof glfwModule.extraCodes, 'object');
	});
});
