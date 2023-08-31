"use strict";
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
async function* Queue(value) {
    while (true)
        value = await new Promise(yield value);
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
        const quqe = Queue();
        quqeMap.set(this, quqe);
        quqe.next();
        quqe.next((res) => {
            this.stdout.once('data', (data) => {
                const [, pid, socket, addr, port] = String(data).match(/^pid=(\d+)(, addr=(\d+\.\d+\.\d+\.\d+:\d+))?/);
                this.pid = Number(pid);
                if (socket) {
                    this.addr = addr;
                    this.port = Number(port);
                }
                super.emit('init', this.pid, this.addr, this.port);
                res();
            });
        });
        super.once('exit', (code) => {
            this.exitCode = code;
            quqeMap.get(this).return(null);
        });
    }
    postMessage(obj) { OCR.prototype.flush.call(this, obj); }
    async flush(obj) {
        return (await quqeMap.get(this).next((res) => {
            super.once('message', res);
            super.postMessage(obj);
        })).value;
    }
}
module.exports = OCR;
