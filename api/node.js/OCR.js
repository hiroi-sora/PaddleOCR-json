const { isMainThread } = require('worker_threads');
const { resolve: path_resolve } = require('path');
if (isMainThread) {
    const { Worker } = require('worker_threads');
    class Queue extends Array {
        constructor() {
            super();
            this.status = false
        }
        shift() {
            this.length && Promise.resolve(this[0]()).then(() => (super.shift(), this.status && this.shift()));
        }
        push(cb) {
            super.push(cb) - 1 || this.status && this.shift();
        }
    }
    class OCR extends Worker {
        #queue
        constructor(path, args, options, debug) {
            if (!path) {
                path = 'PaddleOCR_json.exe';
            }
            debug = !!debug;
            super(__filename, {
                workerData: { path, args, options, debug },
            });
            this.#queue = new Queue();
            super.once('message', (code) => {
                console.log(code);
                this.#queue.status = true;
                this.#queue.shift();
            });
        }
        get length() {
            return this.#queue.length;
        }
        postMessage(obj) {
            return new Promise((res) => {
                const queue = this.#queue;
                obj = Object.assign({}, obj);
                if (obj.image_dir === null) obj.image_dir = 'clipboard'
                else obj.image_dir = path_resolve(obj.image_dir);
                queue.push(() => new Promise((res_) => {
                    super.once('message', (data) => (res({
                        code: data.code,
                        message: data.code - 100 ? data.data : '',
                        data: data.code - 100 ? null : data.data,
                    }), res_()));
                    super.postMessage(obj);
                }));
            });
        }
        flush(obj) {
            return this.postMessage(obj);
        }
    }
    module.exports = OCR;
} else {
    const { parentPort, workerData } = require('worker_threads');
    const { spawn } = require('child_process');
    new Promise((res) => {
        const {
            path,
            args,
            options,
            debug,
        } = workerData;
        const proc = spawn(path, [].concat(args, '--use_debug=0'), options);
        proc.stdout.on('data', function stdout(chunk) {
            if (!chunk.toString().match('OCR init completed.')) return;
            proc.stdout.off('data', stdout);
            return res(proc);
        });
        if (debug) {
            proc.stdout.on('data', (chunk) => {
                console.log(chunk.toString());
            });
            proc.stderr.on('data', (data) => {
                console.log(data.toString());
            });
            proc.on('close', (code) => {
                console.log('close code: ', code);
            });
            proc.on('exit', (code) => {
                console.log('exit code: ', code);
            });
        }
    }).then((proc) => {
        parentPort.postMessage({
            code: 0,
            message: 'OCR init completed.',
            pid: proc.pid,
        });
        parentPort.on('message', (data) => {
            proc.stdin.write(`${JSON.stringify(data)}\n`)
        });
        proc.stdout.on('data', (chunk) => {
            parentPort.postMessage(JSON.parse(chunk));
        });
    });
}