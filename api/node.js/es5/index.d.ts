/// <reference types="node" />
import { Worker } from 'worker_threads';
declare class OCR extends Worker {
    pid: number;
    constructor(path?: string, args?: string[], options?: OCR.Options, debug?: boolean);
    emit(...args: Parameters<typeof Worker.prototype.emit>): boolean;
    postMessage(obj: OCR.DArg): void;
    flush(obj: OCR.DArg): Promise<OCR.coutReturnType>;
}
declare namespace OCR {
    interface DArg {
        image_dir?: string | null;
        limit_side_len?: number;
        limit_type?: string;
        visualize?: boolean;
        output?: string;
    }
    interface coutReturnType {
        code: number;
        message: string;
        pid?: number;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]];
            score: number;
            text: string;
        }[] | null;
    }
    type Options = Omit<import('child_process').SpawnOptionsWithStdioTuple<'pipe', 'pipe', 'pipe'>, keyof typeof import('./worker').__default.options>;
    interface EventMap {
        init: [pid: number];
        message: [data: OCR.coutReturnType];
        error: [err: Error];
        exit: [exitCode: number];
        messageerror: [error: Error];
        online: [];
    }
}
interface OCR extends Worker {
    on<K extends keyof OCR.EventMap>(event: K, listener: (...arg: OCR.EventMap[K]) => void): this;
    on(event: string | symbol, listener: (...args: any[]) => void): this;
    once<K extends keyof OCR.EventMap>(event: K, listener: (...arg: OCR.EventMap[K]) => void): this;
    once(event: string | symbol, listener: (...args: any[]) => void): this;
}
export = OCR;
