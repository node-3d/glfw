import { GlfwWindow, glfw } from '@node-3d/glfw';

const schedulers = ['idle', 'immediate'] as const;
type TScheduler = (typeof schedulers)[number];

const schedulerArg = process.argv
	.find((arg) => arg.startsWith('--scheduler='))
	?.slice('--scheduler='.length);
const scheduler: TScheduler = schedulers.includes(schedulerArg as TScheduler)
	? (schedulerArg as TScheduler)
	: 'idle';

const w1 = new GlfwWindow({ title: `GLFW Perf (${scheduler})`, vsync: false });

let prevTime = Date.now();
let frames = 0;
let events = 0;

w1.on('mousemove', () => {
	events++;
});

const drawFrame = () => {
	if (w1.shouldClose || w1.getKey(glfw.KEY_ESCAPE)) {
		process.exit(0);
		return;
	}

	glfw.testScene(w1.width, w1.height);

	frames++;
	const time = Date.now();
	if (time >= prevTime + 5000) {
		console.log('FPS:', Math.floor((frames * 1000) / (time - prevTime)), 'Events:', events);
		prevTime = time;
		frames = 0;
		events = 0;
	}
};

if (scheduler === 'immediate') {
	const loop = () => {
		w1.drawWindow(drawFrame);
		setImmediate(loop);
	};
	setImmediate(loop);
} else {
	w1.loop(drawFrame);
}
