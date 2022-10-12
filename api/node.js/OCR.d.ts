import { Worker } from 'worker_threads';

import { spawn } from 'child_process';
type ParamType<T> = T extends (command: any, args: any, options: infer P) => any ? P : T;
type SpawnOptions = ParamType<typeof spawn>;
interface Options extends SpawnOptions {
    argv0: undefined,
    stdio: 'pipe',
    detached: false,
    shell: false,
    windowsVerbatimArguments: undefined,
    windowsHide: true,
}

declare class OCR extends Worker {
    constructor(path: string, args?: string[], options?: Options, debug: boolean = false);
    readonly length: number;
    postMessage(obj: {
        image_dir?: string | null,
        limit_side_len?: number = 960,
        limit_type?: string = 'max',
        visualize?: boolean = false,
        output?: string = './output/',
    }): Promise<{
        code: number,
        message: string,
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]],
            score: number,
            text: string,
        }[] | null,
    }>;
    flush: typeof this.postMessage;
}

export = OCR;