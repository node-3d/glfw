import { glfw, Window } from '@node-3d/glfw';

// Default MSAA is 0
const w1 = new Window({ title: 'GLFW MSAA x0', msaa: 0, vsync: true, width: 640, height: 480 });
const w2 = new Window({ title: 'GLFW MSAA x4', msaa: 4, vsync: true, width: 640, height: 480 });

const draw = () => {
	glfw.pollEvents();

	w1.makeCurrent();
	glfw.testScene(w1.width, w1.height);
	w1.swapBuffers();

	w2.makeCurrent();
	glfw.testScene(w2.width, w2.height);
	w2.swapBuffers();
};

const animate = () => {
	if (
		w1.shouldClose ||
		w2.shouldClose ||
		w1.getKey(glfw.KEY_ESCAPE) ||
		w2.getKey(glfw.KEY_ESCAPE)
	) {
		process.exit(0);
		return;
	}

	draw();
	setImmediate(animate);
};

animate();
