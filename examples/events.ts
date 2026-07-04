import { glfw, Window } from '@node-3d/glfw';
import type { TEvent } from '@node-3d/glfw';

const w1 = new Window({ title: 'GLFW Events', vsync: true });

const makeEventLogger = (name: string) => (e: TEvent) => {
	e.target = null;
	console.log(`[${name}]`, e);
};
const eventList = [
	'blur',
	'click',
	'drop',
	'focus',
	'focusin',
	'focusout',
	'resize',
	'iconifiy',
	'keydown',
	'keyup',
	'mousedown',
	'mouseenter',
	'mouseleave',
	'mouseup',
	'quit',
	'refresh',
	'wresize',
	'wheel',
	'move',
	// 'mousemove', // too many events
];

for (const name of eventList) {
	w1.on(name, makeEventLogger(name));
}

w1.loop(() => {
	if (w1.shouldClose || w1.getKey(glfw.KEY_ESCAPE)) {
		process.exit(0);
		return;
	}

	glfw.testScene(w1.width, w1.height);
});
