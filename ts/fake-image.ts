import { getLogger } from '@node-3d/addon-tools';

const logger = getLogger('glfw');

export class FakeImage {
	public get src(): string {
		logger.error('Document.Image class not set.');
		return '';
	}
	
	public set src(_v: string) {
		logger.error('Document.Image class not set.');
	}
	
	public get complete(): boolean { return false; }
	public on(): void { /* nop */ }
	public onload(): void { /* nop */ }
	public onerror(): void { /* nop */ }
}
