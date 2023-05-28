"use strict";
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
class Queue extends Array {
    out() {
        if (!this.length)
            return;
        new Promise((res) => this[0](res))
            .then(() => (super.shift(), this.out()));
    }
    in(fn) {
        super.push(fn) - 1 || this.out();
        return true;
    }
}
const quqeMap = new WeakMap();
class OCR extends worker_threads_1.Worker {
    pid;
    addr;
    port;
    exitCode = null;
    constructor(path, args, options, debug) {
        super((0, path_1.resolve)(__dirname, 'worker.js'), {
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
                super.emit('init', this.pid);
                next();
            });
        });
        super.once('exit', (code) => {
            this.exitCode = code;
            quqeMap.delete(this);
        });
    }
    postMessage(obj) {
        quqeMap.get(this)?.in((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj) {
        return new Promise((res) => {
            quqeMap.get(this).in((next) => {
                super.once('message', (data) => {
                    res(data);
                    next();
                });
                super.postMessage(obj);
            });
        });
    }
}
module.exports = OCR;
