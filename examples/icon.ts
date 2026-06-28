import { glfw, Window } from '@node-3d/glfw';


const w1 = new Window({ title: 'GLFW Icon', vsync: true });

const ICON_SIZE = 32;

const iconData = [];
for (let y = 0; y < ICON_SIZE; y++) {
	for (let x = 0; x < ICON_SIZE; x++) {
		const r = 255 * (x / (ICON_SIZE - 1));
		const g = 255 * (y / (ICON_SIZE - 1));
		const b = 255 * (1 - r * g);
		iconData.push(r, g, b, 255);
	}
}

const icon = {
	width  : ICON_SIZE,
	height : ICON_SIZE,
	data   : Buffer.from(iconData),
	noflip : true,
};

w1.icon = icon;


w1.loop(() => {
	if (w1.shouldClose || w1.getKey(glfw.KEY_ESCAPE)) {
		process.exit(0);
		return;
	}
	
	glfw.testScene(w1.width, w1.height);
});
