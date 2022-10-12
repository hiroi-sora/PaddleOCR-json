import { Worker } from 'worker_threads';

declare class OCR extends Worker {
    constructor(path: string, args?: string[], options?: object, debug: boolean = false);
    get length(): number;
    postMessage(obj: {
        image_dir?: string | null,
        limit_side_len: number = 960,
        limit_type: string = 'max',
        visualize: boolean = false,
        output: string = './output/',
    }): Promise<{
        code: number,
        message: string,
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]],
            score: number,
            text: string,
        }[],
    }>;
    flush: typeof this.postMessage;
}