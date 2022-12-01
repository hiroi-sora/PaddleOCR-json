import { Worker } from 'worker_threads';
import { resolve as path_resolve } from 'path';
import { DArg, __default, Options, coutReturnType } from './worker';

class Queue extends Array<(next: () => void) => any> {
    status: boolean = false;
    shift(): any {
        if (this.length) new Promise((res: (v?: any) => void) => this[0](res)).then(() => (super.shift(), this.status && this.shift()));
    }
    push(fn: (next: () => void) => any): any {
        super.push(fn) - 1 || this.status && this.shift();
    }
}
class OCR extends Worker {
    #queue: Queue
    constructor(path?: string, args?: string[], options?: Options, debug: boolean = false) {
        if (path && / /.test(path)) path = JSON.stringify(path);
        super(path_resolve(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
        });
        const queue = this.#queue = new Queue();
        super.once('message', (data) => (super.emit('init', data.pid), queue.status = true, queue.shift()));
    }
    get length() {
        return this.#queue.length;
    }
    emit(...args: Parameters<typeof Worker.prototype.emit>) {
        if (args[0] === 'init') return false;
        return super.emit(...args);
    }
    postMessage(obj: DArg) {
        this.#queue.push((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj: DArg) {
        return new Promise((res: (v: coutReturnType) => void) => {
            this.#queue.push((next) => {
                super.once('message', (data: coutReturnType) => (res(data), next()));
                super.postMessage(obj);
            });
        });
    }
}

export = OCR;