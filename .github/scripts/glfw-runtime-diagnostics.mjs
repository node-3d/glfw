// oxlint-disable no-console node/no-process-env node/no-sync
import { spawnSync } from 'node:child_process';

const reportPrefix = '__NODE_3D_GLFW_DIAG__';
const nativeUrl = new URL('../../ts/native.ts', import.meta.url).href;
const modes = [
	'default',
	'native-loose',
	'null-native',
	'null-egl',
	'null-osmesa',
	'gles-egl',
	'angle-d3d11',
	'angle-vulkan',
	'angle-metal',
];

const childSource = String.raw`
const reportPrefix = '${reportPrefix}';

const asMessage = (error) => error instanceof Error ? error.name + ': ' + error.message : String(error);

const writeReport = (report) => {
	console.log(reportPrefix + JSON.stringify(report));
};

try {
	const { native: glfw } = await import(process.env.NODE_3D_GLFW_NATIVE_URL);
	const result = glfw.testHeadlessFboMode(process.env.NODE_3D_GLFW_FBO_MODE, 16, 16);
	const pixels = result.pixels;
	writeReport({
		ok: true,
		details: {
			mode: result.mode,
			platform: result.platform,
			width: result.width,
			height: result.height,
			status: result.status,
			firstPixel: [pixels[0], pixels[1], pixels[2], pixels[3]],
			vendor: result.vendor,
			renderer: result.renderer,
			version: result.version,
		},
	});
} catch (error) {
	writeReport({ ok: false, error: asMessage(error) });
	process.exitCode = 1;
}
`;

const readReport = (stdout) => {
	const line = stdout.split(/\r?\n/u).find((currentLine) => currentLine.startsWith(reportPrefix));

	if (!line) {
		return null;
	}

	return JSON.parse(line.slice(reportPrefix.length));
};

const uniq = (values) => [...new Set(values)];

const extractGlfwErrors = (output) =>
	uniq(
		output
			.split(/\r?\n/u)
			.map((line) => line.trim())
			.filter((line) => line.startsWith('GLFW Error ')),
	);

const exitText = (child) => {
	if (child.error) {
		return child.error.message;
	}

	if (child.signal) {
		return `signal ${child.signal}`;
	}

	return `exit ${child.status}`;
};

console.log(`[glfw-diag] node ${process.version} ${process.platform} ${process.arch}`);

for (const mode of modes) {
	const child = spawnSync(process.execPath, ['--input-type=module', '--eval', childSource], {
		encoding: 'utf8',
		env: {
			...process.env,
			NODE_3D_GLFW_FBO_MODE: mode,
			NODE_3D_GLFW_NATIVE_URL: nativeUrl,
		},
	});
	const stdout = child.stdout || '';
	const stderr = child.stderr || '';
	const report = readReport(stdout);

	if (child.status === 0 && report?.ok) {
		console.log(`[glfw-diag] ok ${mode} ${JSON.stringify(report.details)}`);
	} else {
		const glfwErrors = extractGlfwErrors(`${stdout}\n${stderr}`);
		const reason = report?.error || exitText(child);
		const glfwText = glfwErrors.length > 0 ? `; ${glfwErrors.join(' | ')}` : '';
		console.log(`[glfw-diag] fail ${mode} ${reason}${glfwText}`);
	}
}
