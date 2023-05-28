import { Worker } from 'worker_threads';
import { resolve as path_resolve } from 'path';

const $quqe = (() => {
    class Queue extends Array<(next: () => void) => any> {
        out() {
            if (!this.length) return;
            new Promise((res: (v?: any) => void) => this[0](res))
                .then(() => (super.shift(), this.out()));
        }
        in(fn: (next: () => void) => any) {
            super.push(fn) - 1 || this.out();
        }
    }
    const map = new WeakMap<OCR, Queue>();
    return (key: OCR) => {
        let value = map.get(key);
        if (value) return value;
        value = new Queue();
        map.set(key, value);
        return value;
    };
})();

class OCR extends Worker {
    pid: number;
    constructor(path?: string, args?: string[], options?: OCR.Options) {
        super(path_resolve(__dirname, 'worker.js'), {
            workerData: { path, args, options },
            stdout: true,
        });
        $quqe(this).in((next) => {
            this.stdout.read()
            this.stdout.once('data', (pid) => {
                super.emit('init', this.pid = Number(String(pid)));
                next();
            });
        });
    }
    postMessage(obj: OCR.DArg) {
        $quqe(this).in((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj: OCR.DArg) {
        return new Promise((res: (v: OCR.coutReturnType) => void) =>
            $quqe(this).in((next) => {
                super.once('message', (data: OCR.coutReturnType) => {
                    res(data);
                    next();
                });
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
}

export = OCR;
