import { Worker } from 'worker_threads';
import { resolve as path_resolve } from 'path';

const $quqe = (() => {
    class Queue extends Array<(next: () => void) => any> {
        shift(): any {
            if (this.length) new Promise((res: (v?: any) => void) => this[0](res)).then(() => (super.shift(), this.shift()));
        }
        push(fn: (next: () => void) => any): any {
            super.push(fn) - 1 || this.shift();
        }
    }
    const map = new class extends WeakMap<OCR, Queue> {
        get(key: OCR) {
            return super.get(key) || ((quqe) => (
                super.set(key, quqe), quqe
            ))(new Queue());
        }
    }();
    return map.get.bind(map) as typeof map.get;
})();

class OCR extends Worker {
    pid: number;
    constructor(path?: string, args?: string[], options?: OCR.Options, debug: boolean = false) {
        if (path && / /.test(path)) path = JSON.stringify(path);
        super(path_resolve(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
        });
        $quqe(this).push((next) =>
            super.once('message', (data) => (
                super.emit('init', this.pid = data.pid), next()
            ))
        );
    }
    emit(...args: Parameters<typeof Worker.prototype.emit>) {
        if (args[0] === 'init') return false;
        return super.emit(...args);
    }
    postMessage(obj: OCR.DArg) {
        $quqe(this).push((next) => {
            super.once('message', next)
            super.postMessage(obj);
        });
    }
    flush(obj: OCR.DArg) {
        return new Promise((res: (v: OCR.coutReturnType) => void) =>
            $quqe(this).push((next) => {
                super.once('message', (data: OCR.coutReturnType) => (
                    res(data), next()
                ));
                super.postMessage(obj);
            })
        );
    }
}

namespace OCR {

    export interface DArg {
        image_dir?: string | null;
        limit_side_len?: number;
        limit_type?: string;
        visualize?: boolean;
        output?: string;
    }

    export interface coutReturnType {
        code: number;
        message: string;
        pid?: number;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]],
            score: number,
            text: string,
        }[] | null;
    }

    export type Options = Omit<
        import('child_process').SpawnOptionsWithStdioTuple<'pipe', 'pipe', 'pipe'>,
        keyof typeof import('./worker').__default.options
    >;

    export interface EventMap {
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