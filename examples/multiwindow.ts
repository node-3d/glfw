import { GlfwWindow, glfw } from '@node-3d/glfw';
import { setIdleLoop } from '@node-3d/uv-loop';
import type { TMouseMoveEvent } from '../ts';

const windows = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].map((i) => {
	const w = new GlfwWindow({
		title: `GLFW Multiwindow ${i}`,
		vsync: false,
		width: 200,
		height: 200,
	});
	const render = () => {
		w.makeCurrent();
		glfw.testScene(w.width, w.height);
	};
	return { w, render };
});

// testing events
for (let i = 0; i < windows.length; i++) {
	const wnd = windows[i];
	if (!wnd) {
		continue;
	}
	wnd.w.on('mousemove', (e: TMouseMoveEvent) =>
		console.log(`[#${i + 1} mousemove] ${e.x}, ${e.y}`),
	);
}

let prevTime = Date.now();
let frames = 0;
let events = 0;

const loopFunc = () => {
	for (const window of windows) {
		const { w, render } = window;
		if (w.shouldClose || w.getKey(glfw.KEY_ESCAPE)) {
			process.exit(0);
		}
		w.drawWindow(render);
	}

	frames++;
	const time = Date.now();
	if (time >= prevTime + 5000) {
		console.log('FPS:', Math.floor((frames * 1000) / (time - prevTime)), 'Events:', events);
		prevTime = time;
		frames = 0;
		events = 0;
	}
};

setIdleLoop(loopFunc);
