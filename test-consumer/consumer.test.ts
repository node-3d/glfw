import assert from 'node:assert/strict';
import test from 'node:test';
import { GlfwWindow, glfw } from '@node-3d/glfw';

test('initializes the packed GLFW addon', () => {
	assert.equal(typeof GlfwWindow, 'function');
	assert.equal(typeof glfw.createWindow, 'function');
	assert.ok(glfw.VERSION_MAJOR >= 3);
});
