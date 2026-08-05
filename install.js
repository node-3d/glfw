import { install } from '@node-3d/addon-tools';

const prefix = 'https://github.com/node-3d/glfw/releases/download';
const tag = '7.1.1';

await install(`${prefix}/${tag}`);
