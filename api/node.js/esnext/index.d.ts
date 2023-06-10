/// <reference types="node" />
import { Worker } from 'worker_threads';
declare class OCR extends Worker {
    pid: number;
    addr: string | undefined;
    port: number | undefined;
    exitCode: number | null;
    constructor(path?: string, args?: string[], options?: OCR.Options, debug?: boolean);
    postMessage(obj: OCR.Arg): void;
    flush(obj: OCR.Arg): Promise<OCR.coutReturnType>;
}
declare namespace OCR {
    interface BaseArg {
        limit_side_len?: number;
        limit_type?: string;
        visualize?: boolean;
        output?: string;
    }
    interface Arg_Path extends BaseArg {
        image_path?: string | null;
    }
    interface Arg_Base64 extends BaseArg {
        image_base64?: string;
    }
    export type Arg = Arg_Base64 | Arg_Path;
    export interface coutReturnType {
        code: number;
        message: string;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]];
            score: number;
            text: string;
        }[] | null;
    }
    export type Options = Omit<import('child_process').SpawnOptionsWithStdioTuple<'pipe', 'pipe', 'pipe'>, keyof typeof import('./worker').__default.options>;
    export {};
}
export = OCR;
