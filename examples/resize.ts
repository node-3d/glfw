import { glfw, Window } from '@node-3d/glfw';


const w1 = new Window({ title: 'GLFW Resize', vsync: true });

// testing events
w1.on('keydown', () => { w1.height += 10; });
w1.on('mousedown', () => { w1.width += 10; });

console.log('Press a key to resize Height.\nClick mouse to resize Width.');
w1.on(
	'resize',
	({ width, height}) => console.log('Resized to', width, 'x', height)
);


w1.loop(() => {
	if (w1.shouldClose || w1.getKey(glfw.KEY_ESCAPE)) {
		process.exit(0);
		return;
	}
	
	glfw.testScene(w1.width, w1.height);
});
