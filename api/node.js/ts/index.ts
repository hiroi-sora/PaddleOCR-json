import { Worker } from 'worker_threads';
import { resolve as path_resolve } from 'path';

class Queue extends Array<(next: () => void) => any> {
    out() {
        if (!this.length) return;
        new Promise((res: (v?: any) => void) => this[0](res))
            .then(() => (super.shift(), this.out()));
    }
    in(fn: (next: () => void) => any) {
        super.push(fn) - 1 || this.out();
        return true;
    }
}
const quqeMap = new WeakMap<OCR, Queue>();

class OCR extends Worker {
    pid: number;
    addr: string | undefined;
    port: number | undefined;
    exitCode: number | null = null;
    constructor(path?: string, args?: string[], options?: OCR.Options, debug?: boolean) {
        super(path_resolve(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
            stdout: true,
        });
        const quqe = new Queue();
        quqeMap.set(this, quqe);
        quqe.in((next) => {
            this.stdout.read();
            this.stdout.once('data', (data) => {
                const [, pid, , addr, port] = String(data).match(/^pid=(\d+), socket=((\d+\.\d+\.\d+\.\d+):(\d+))?/);
                this.pid = Number(pid);
                this.addr = addr;
                this.port = Number(port);
                super.emit('init', this.pid, this.addr, this.port);
                next();
            });
        });
        super.once('exit', (code) => {
            this.exitCode = code;
            quqeMap.delete(this);
        });
    }
    postMessage(obj: OCR.DArg) {
        quqeMap.get(this)?.in((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj: OCR.DArg) {
        return new Promise((res: (v: OCR.coutReturnType) => void) => {
            quqeMap.get(this).in((next) => {
                super.once('message', (data: OCR.coutReturnType) => {
                    res(data);
                    next();
                });
                super.postMessage(obj);
            });
        });
    }
}

namespace OCR {

    export interface DArg {
        image_path?: string | null;
        image_base64?: string;
        limit_side_len?: number;
        limit_type?: string;
        visualize?: boolean;
        output?: string;
        /* ... */
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
