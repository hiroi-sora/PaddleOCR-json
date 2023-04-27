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
    return (key) => map.get(key) || ((quqe) => (map.set(key, quqe), quqe))(new Queue());
})();
class OCR extends worker_threads_1.Worker {
    pid;
    constructor(path, args, options, debug = false) {
        super((0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
            stdout: true,
        });
        $quqe(this).in((next) => this.stdout.once('data', (pid) => {
            super.emit('init', this.pid = Number(pid));
            next();
        }));
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
