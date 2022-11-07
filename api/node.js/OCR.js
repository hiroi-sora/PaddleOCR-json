"use strict";
const { isMainThread } = require('worker_threads');
if (isMainThread) {
    const { Worker } = require('worker_threads');
    class Queue extends Array {
        status = false;
        shift() {
            this.length && new Promise((res) => this[0](res)).then(() => (super.shift(), this.status && this.shift()));
            return this;
        }
        push(fn) {
            super.push(fn) - 1 || this.status && this.shift();
        }
    }
    class OCR extends Worker {
        #queue;
        constructor(path, args, options, debug = false) {
            if (path && / /.test(path)) path = JSON.stringify(path);
            super(__filename, { workerData: { path, args, options, debug } });
            const queue = this.#queue = new Queue();
            super.once('message', () => queue.shift().status = true);
        }
        get length() {
            return this.#queue.length;
        }
        postMessage(obj) {
            return new Promise((res) => {
                this.#queue.push((next) => {
                    super.once('message', (data) => (res(data), next()));
                    super.postMessage(obj);
                });
            });
        }
        flush(obj) {
            return this.postMessage(obj);
        }
    }
    module.exports = OCR;
} else {
    const __default = {
        path: 'PaddleOCR_json.exe',
        args: ['--use_debug=0'],
        options: {
            argv0: undefined,
            stdio: 'pipe',
            detached: false,
            shell: false,
            windowsVerbatimArguments: undefined,
            windowsHide: true,
        },
        initTag: 'OCR init completed.',
    };
    const { parentPort, workerData } = require('worker_threads');
    const { spawn } = require('child_process');
    const { resolve: path_resolve } = require('path');
    new Promise((res) => {
        const { path = __default.path, args = [], options, debug = false, } = workerData;
        const proc = spawn(path, args.concat(__default.args), Object.assign({}, options, __default.options));
        proc.stdout.on('data', function stdout(chunk) {
            if (!chunk.toString().match(__default.initTag)) return;
            proc.stdout.off('data', stdout);
            return res(proc);
        });
        if (debug) {
            proc.stdout.on('data', (chunk) => console.log(chunk.toString()));
            proc.stderr.on('data', (data) => console.log(data.toString()));
            proc.on('close', (code) => console.log('close code: ', code));
            proc.on('exit', (code) => console.log('exit code: ', code));
        }
    }).then((proc) => {
        parentPort.postMessage({
            code: 0,
            message: __default.initTag,
            pid: proc.pid,
        });
        parentPort.on('message', (data) => proc.stdin.write(`${JSON.stringify(cargs(data))}\n`));
        proc.stdout.on('data', (chunk) => parentPort.postMessage(cout(JSON.parse(chunk))));
    });
    function cargs(obj) {
        obj = Object.assign({}, obj);
        if (obj.image_dir === null) obj.image_dir = 'clipboard';
        else if (obj.image_dir) obj.image_dir = (0, path_resolve)(obj.image_dir);
        return obj;
    }
    function cout(data) {
        return {
            code: data.code,
            message: data.code - 100 ? data.data : '',
            data: data.code - 100 ? null : data.data,
        };
    }
}
