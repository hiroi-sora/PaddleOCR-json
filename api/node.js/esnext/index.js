"use strict";
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
const $quqe = (() => {
    class Queue extends Array {
        shift() {
            if (this.length)
                new Promise((res) => this[0](res)).then(() => (super.shift(), this.shift()));
        }
        push(fn) {
            super.push(fn) - 1 || this.shift();
        }
    }
    const map = new class extends WeakMap {
        get(key) {
            return super.get(key) || ((quqe) => (super.set(key, quqe), quqe))(new Queue());
        }
    }();
    return map.get.bind(map);
})();
class OCR extends worker_threads_1.Worker {
    pid;
    constructor(path, args, options, debug = false) {
        if (path && / /.test(path))
            path = JSON.stringify(path);
        super((0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
        });
        $quqe(this).push((next) => super.once('message', (data) => (super.emit('init', this.pid = data.pid), next())));
    }
    emit(...args) {
        if (args[0] === 'init')
            return false;
        return super.emit(...args);
    }
    postMessage(obj) {
        $quqe(this).push((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj) {
        return new Promise((res) => $quqe(this).push((next) => {
            super.once('message', (data) => (res(data), next()));
            super.postMessage(obj);
        }));
    }
}
module.exports = OCR;
