import { glfw, Window } from '@node-3d/glfw';
import type { TWindowMode } from '@node-3d/glfw';

let mode: TWindowMode = 'windowed';

if (process.argv.includes('--fullscreen')) {
	mode = 'fullscreen';
} else if (process.argv.includes('--borderless')) {
	mode = 'borderless';
}

const w1 = new Window({ title: 'GLFW Modes', mode, vsync: true });

w1.on('resize', ({ width, height }) => console.log('Resized to', width, 'x', height));

w1.on('keydown', (e) => {
	if (e.keyCode === glfw.KEY_F && e.ctrlKey && e.shiftKey) {
		w1.mode = 'windowed';
	} else if (e.keyCode === glfw.KEY_F && e.ctrlKey && e.altKey) {
		w1.mode = 'fullscreen';
	} else if (e.keyCode === glfw.KEY_F && e.ctrlKey) {
		w1.mode = 'borderless';
	}
});

w1.loop(() => {
	if (w1.shouldClose || w1.getKey(glfw.KEY_ESCAPE)) {
		process.exit(0);
		return;
	}

	glfw.testScene(w1.width, w1.height);
});
