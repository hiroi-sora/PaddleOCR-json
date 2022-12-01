"use strict";
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
class Queue extends Array {
    status = false;
    shift() {
        if (this.length)
            new Promise((res) => this[0](res)).then(() => (super.shift(), this.status && this.shift()));
    }
    push(fn) {
        super.push(fn) - 1 || this.status && this.shift();
    }
}
class OCR extends worker_threads_1.Worker {
    #queue;
    constructor(path, args, options, debug = false) {
        if (path && / /.test(path))
            path = JSON.stringify(path);
        super((0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
        });
        const queue = this.#queue = new Queue();
        super.once('message', (data) => (super.emit('init', data.pid), queue.status = true, queue.shift()));
    }
    get length() {
        return this.#queue.length;
    }
    emit(...args) {
        if (args[0] === 'init')
            return false;
        return super.emit(...args);
    }
    postMessage(obj) {
        this.#queue.push((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj) {
        return new Promise((res) => {
            this.#queue.push((next) => {
                super.once('message', (data) => (res(data), next()));
                super.postMessage(obj);
            });
        });
    }
}
module.exports = OCR;
