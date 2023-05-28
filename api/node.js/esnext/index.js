"use strict";
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
const $quqe = (() => {
    class Queue extends Array {
        out() {
            if (!this.length)
                return;
            new Promise((res) => this[0](res))
                .then(() => (super.shift(), this.out()));
        }
        in(fn) {
            super.push(fn) - 1 || this.out();
        }
    }
    const map = new WeakMap();
    return (key) => {
        let value = map.get(key);
        if (value)
            return value;
        value = new Queue();
        map.set(key, value);
        return value;
    };
})();
class OCR extends worker_threads_1.Worker {
    pid;
    constructor(path, args, options) {
        super((0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path, args, options },
            stdout: true,
        });
        $quqe(this).in((next) => {
            this.stdout.read();
            this.stdout.once('data', (pid) => {
                super.emit('init', this.pid = Number(String(pid)));
                next();
            });
        });
    }
    postMessage(obj) {
        $quqe(this).in((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj) {
        return new Promise((res) => $quqe(this).in((next) => {
            super.once('message', (data) => {
                res(data);
                next();
            });
            super.postMessage(obj);
        }));
    }
}
module.exports = OCR;
