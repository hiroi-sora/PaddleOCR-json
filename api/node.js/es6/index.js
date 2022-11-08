"use strict";
var __classPrivateFieldSet = (this && this.__classPrivateFieldSet) || function (receiver, state, value, kind, f) {
    if (kind === "m") throw new TypeError("Private method is not writable");
    if (kind === "a" && !f) throw new TypeError("Private accessor was defined without a setter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver)) throw new TypeError("Cannot write private member to an object whose class did not declare it");
    return (kind === "a" ? f.call(receiver, value) : f ? f.value = value : state.set(receiver, value)), value;
};
var __classPrivateFieldGet = (this && this.__classPrivateFieldGet) || function (receiver, state, kind, f) {
    if (kind === "a" && !f) throw new TypeError("Private accessor was defined without a getter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver)) throw new TypeError("Cannot read private member from an object whose class did not declare it");
    return kind === "m" ? f : kind === "a" ? f.call(receiver) : f ? f.value : state.get(receiver);
};
var _OCR_queue;
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
class Queue extends Array {
    constructor() {
        super(...arguments);
        this.status = false;
    }
    shift() {
        if (this.length)
            new Promise((res) => this[0](res)).then(() => (super.shift(), this.status && this.shift()));
    }
    push(fn) {
        super.push(fn) - 1 || this.status && this.shift();
    }
}
class OCR extends worker_threads_1.Worker {
    constructor(path, args, options, debug = false) {
        if (path && / /.test(path))
            path = JSON.stringify(path);
        super((0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
        });
        _OCR_queue.set(this, void 0);
        const queue = __classPrivateFieldSet(this, _OCR_queue, new Queue(), "f");
        super.once('message', (data) => (super.emit('init', data.pid), queue.status = true, queue.shift()));
    }
    get length() {
        return __classPrivateFieldGet(this, _OCR_queue, "f").length;
    }
    emit(...args) {
        if (args[0] === 'init')
            return false;
        return super.emit(...args);
    }
    postMessage(obj) {
        __classPrivateFieldGet(this, _OCR_queue, "f").push((next) => {
            super.once('message', next);
            super.postMessage(obj);
        });
    }
    flush(obj) {
        return new Promise((res) => {
            __classPrivateFieldGet(this, _OCR_queue, "f").push((next) => {
                super.once('message', (data) => (res(data), next()));
                super.postMessage(obj);
            });
        });
    }
}
_OCR_queue = new WeakMap();
module.exports = OCR;
