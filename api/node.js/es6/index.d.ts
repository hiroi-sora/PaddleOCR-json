/// <reference types="node" />
import { Worker } from 'worker_threads';
import { DArg, Options } from './worker';
declare class OCR extends Worker {
    #private;
    constructor(path?: string, args?: string[], options?: Options, debug?: boolean);
    get length(): number;
    emit(...args: Parameters<typeof Worker.prototype.emit>): boolean;
    postMessage(obj: DArg): void;
    flush(obj: DArg): Promise<{
        code: number;
        message: string;
        pid?: number;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]];
            score: number;
            text: string;
        }[];
    }>;
}
export = OCR;
